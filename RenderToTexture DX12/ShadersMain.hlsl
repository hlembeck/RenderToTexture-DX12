Texture2D<float> gTexture : register(t0);
SamplerState gSampler : register(s0);

float4 vsMain(float3 pos : POSITION) : SV_POSITION{
	return float4(pos,1.0f);
}

float4 psMain(float4 pos : SV_POSITION) : SV_TARGET{
	return float4(gTexture.Sample(gSampler, pos.xy),0.0f,0.0f,1.0f);
}