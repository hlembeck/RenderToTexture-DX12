float4 vsMain(float2 pos : POSITION) : SV_POSITION{
	return float4(pos,0.0f,1.0f);
}

float psMain(float4 pos : SV_POSITION) : SV_TARGET{
	return 1.0f;
}