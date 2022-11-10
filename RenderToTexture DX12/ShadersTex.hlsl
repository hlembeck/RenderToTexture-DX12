//struct VSOutput {
//	float4 pos : SV_POSITION;
//	uint slice : SLICE;
//};
//
//struct GSOutput {
//	float4 pos : SV_POSITION;
//	uint slice : SV_RenderTargetArrayIndex;
//};

float4 vsMain(float2 pos : POSITION) : SV_POSITION{
	return float4(pos,0.0f,1.0f);
}

float psMain(float4 pos : SV_POSITION) : SV_TARGET{
	return pos.x/16.0f;
}