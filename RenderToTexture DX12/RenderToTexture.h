#pragma once
#include "Shared.h"

constexpr UINT TEXTURE_RESOLUTION = 16;

//Learning how to render to a texture, which is then used as SRV in the main pass.
class RenderToTexture {
public:
	RenderToTexture();
	RenderToTexture(ID3D12Device* device);

	void FillCommandList(ID3D12GraphicsCommandList* commandList);
	void CreatePipelineState(ID3D12Device* device, ID3D12RootSignature* rootSignature);
	void CreateSRV(ID3D12Device* device, ID3D12DescriptorHeap* srvHeap);
private:
	void LoadVertices(ID3D12Device* device);
	void LoadTexture(ID3D12Device* device);
	ComPtr<ID3D12Resource> m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

	ComPtr<ID3D12Resource> m_texture;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;

	D3D12_RECT m_scissorRect;
	D3D12_VIEWPORT m_viewport;

	ComPtr<ID3D12PipelineState> m_pipelineState;
};