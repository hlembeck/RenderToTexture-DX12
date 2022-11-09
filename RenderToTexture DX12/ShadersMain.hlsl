Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

float4 vsMain(float3 pos : POSITION) : SV_POSITION{
	return float4(pos,1.0f);
}

float4 psMain(float4 pos : SV_POSITION) : SV_TARGET{
	return gTexture.Sample(gSampler, pos.xy);
}