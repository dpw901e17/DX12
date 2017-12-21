struct VS_INPUT
{
	float4 pos : POSITION;
	float2 texCoord: TEXCOORD;
};

struct VS_OUTPUT
{
	float4 pos: SV_POSITION;
	float2 texCoord: TEXCOORD;
};

cbuffer ConstantBuffer : register(b0)
{
	float4x4 worldMat;
};

cbuffer ConstantBuffer : register(b1)
{
	float4x4 viewMat;
};

cbuffer ConstantBuffer : register(b2)
{
	float4x4 projectionMat;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;

	output.pos = mul(input.pos, mul(worldMat, mul(viewMat, projectionMat)));
	output.texCoord = input.texCoord;
	return output;
}