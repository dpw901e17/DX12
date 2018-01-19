Texture2D t1: register(t0);
SamplerState s1: register(s0);

struct VS_OUTPUT
{
	float4 pos: SV_POSITION;
	float2 texCoord: TEXCOORD;
	float4 posCopy: POSITION;
};

float4 main(VS_OUTPUT input) : SV_TARGET
{

	return float4((input.posCopy.x + 4) * 0.2, (input.posCopy.y + 5) * 0.1, (input.posCopy.z + 2) * 0.2, 1.0f) * max(0.1, sin(input.posCopy.x * 30) * sin(input.posCopy.y * 30) * sin(input.posCopy.z * 30));

//normalize(vec4((modelPos.x + 4) * 0.2, (modelPos.y + 5) * 0.1, (modelPos.z + 2) * 0.2, 1.0)) * max(0.1, sin(modelPos.x * c) * sin(modelPos.y * c) * sin(modelPos.z * c))"
}