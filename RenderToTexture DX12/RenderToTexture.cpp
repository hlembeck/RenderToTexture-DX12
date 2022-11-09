#include "RenderToTexture.h"

RenderToTexture::RenderToTexture() {}

RenderToTexture::RenderToTexture(ID3D12Device* device) : m_scissorRect({ 0, 0, TEXTURE_RESOLUTION,TEXTURE_RESOLUTION }), m_viewport({ 0.0f,0.0f,(float)TEXTURE_RESOLUTION,(float)TEXTURE_RESOLUTION,0.0f,1.0f }) {
	LoadVertices(device);
	LoadTexture(device);
}

void RenderToTexture::LoadVertices(ID3D12Device* device) {
	XMFLOAT3 vertices[] = {
		{-1.0f,-1.0f,0.0f},
		{-1.0f,1.0f,0.0f},
		{1.0f,-1.0f,0.0f},

		{1.0f,1.0f,0.0f},
		{1.0f,-1.0f,0.0f},
		{-1.0f,1.0f,0.0f},
	};

	UINT bufSize = sizeof(vertices);
	CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bufSize);
	ThrowIfFailed(device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_vertexBuffer)
	));
	m_vertexBuffer->SetName(L"Vertex Buffer");
	UINT8* pVertexDataBegin;
	CD3DX12_RANGE readRange(0, 0);
	ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
	memcpy(pVertexDataBegin, vertices, bufSize);
	m_vertexBuffer->Unmap(0, nullptr);

	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_vertexBufferView.SizeInBytes = bufSize;
}

void RenderToTexture::LoadTexture(ID3D12Device* device) {
	D3D12_RESOURCE_DESC resourceDesc = {
		D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		0,
		TEXTURE_RESOLUTION,
		TEXTURE_RESOLUTION,
		1,
		1,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		{1,0},
		D3D12_TEXTURE_LAYOUT_UNKNOWN,
		D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
	};

	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
	D3D12_CLEAR_VALUE optimalClearVal;
	optimalClearVal.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	optimalClearVal.Color[0] = 0.0f;
	optimalClearVal.Color[1] = 0.0f;
	optimalClearVal.Color[2] = 0.0f;
	optimalClearVal.Color[3] = 0.0f;
	ThrowIfFailed(device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&optimalClearVal,
		IID_PPV_ARGS(&m_texture)
	));

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		1,
		D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		0
	};

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		D3D12_RTV_DIMENSION_TEXTURE2D
	};

	D3D12_TEX2D_RTV texRTV = {
		0,
		0
	};

	rtvDesc.Texture2D = texRTV;

	device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_rtvHeap));
	device->CreateRenderTargetView(m_texture.Get(), &rtvDesc, m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
}

void RenderToTexture::FillCommandList(ID3D12GraphicsCommandList* commandList) {
	commandList->SetPipelineState(m_pipelineState.Get());

	CD3DX12_RESOURCE_BARRIER resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(m_texture.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandList->ResourceBarrier(1, &resourceBarrier);

	commandList->RSSetViewports(1, &m_viewport);
	commandList->RSSetScissorRects(1, &m_scissorRect);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, NULL);

	const float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	commandList->DrawInstanced(6, 1, 0, 0);

	resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(m_texture.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	commandList->ResourceBarrier(1, &resourceBarrier);
}

void RenderToTexture::CreateSRV(ID3D12Device* device, ID3D12DescriptorHeap* srvHeap) {
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		D3D12_SRV_DIMENSION_TEXTURE2D,
		D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING
	};

	D3D12_TEX2D_SRV tex = {
		0,
		1,
		0,
		0.0f
	};

	srvDesc.Texture2D = tex;

	device->CreateShaderResourceView(m_texture.Get(), &srvDesc, srvHeap->GetCPUDescriptorHandleForHeapStart());
}

void RenderToTexture::CreatePipelineState(ID3D12Device* device, ID3D12RootSignature* rootSignature) {
	//Compile and load shaders
	ComPtr<ID3DBlob> vertexShader;
	ComPtr<ID3DBlob> pixelShader;
	UINT compileFlags = 0;


	ThrowIfFailed(D3DCompileFromFile(L"ShadersTex.hlsl", NULL, NULL, "vsMain", "vs_5_1", compileFlags, 0, &vertexShader, NULL));
	ThrowIfFailed(D3DCompileFromFile(L"ShadersTex.hlsl", NULL, NULL, "psMain", "ps_5_1", compileFlags, 0, &pixelShader, NULL));

	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psDesc = {};
	psDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
	psDesc.pRootSignature = rootSignature;
	psDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
	psDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
	psDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psDesc.SampleMask = UINT_MAX;
	psDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psDesc.NumRenderTargets = 1;
	psDesc.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
	psDesc.SampleDesc.Count = 1;


	device->CreateGraphicsPipelineState(&psDesc, IID_PPV_ARGS(&m_pipelineState));

}