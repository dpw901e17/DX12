/*struct VS_OUTPUT
{
	float4 pos: SV_POSITION;
	float4 color: COLOR;
};

float4 main(VS_OUTPUT input) : SV_TARGET
{
	return input.color;
}*/

float4 main() : SV_TARGET
{
	return float4(0.0f, 1.0f, 0.0f, 1.0f);
}