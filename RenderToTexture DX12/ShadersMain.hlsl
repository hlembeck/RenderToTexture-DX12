Texture3D<float> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct VSOutput {
	float4 pos : SV_POSITION;
	float4 nPos : POSITION;
};

VSOutput vsMain(float3 pos : POSITION) {
	VSOutput ret = (VSOutput)0;
	ret.pos = float4(pos, 1.0f);
	ret.nPos = float4((pos.x+1.0f)*0.5f,(pos.y+1.0f)*0.5f,pos.z,1.0f);
	return ret;
}

float4 psMain(VSOutput input) : SV_TARGET{
	float val = gTexture.Sample(gSampler, input.nPos.xyz);
	return float4(val, 0.0f, 0.0f, 1.0f);
}