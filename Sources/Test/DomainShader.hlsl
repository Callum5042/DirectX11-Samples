struct DomainInputType
{
	float3 position : POSITION;
	float4 color : COLOR;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

// Output patch constant data.
struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[3]			: SV_TessFactor; // e.g. would be [4] for a quad domain
	float InsideTessFactor			: SV_InsideTessFactor; // e.g. would be Inside[2] for a quad domain
};

#define NUM_CONTROL_POINTS 3

cbuffer WorldBuffer : register(b0)
{
	matrix cWorld;
	matrix cView;
	matrix cProjection;
}

[domain("tri")]
PixelInputType main(HS_CONSTANT_DATA_OUTPUT input, float3 domain : SV_DomainLocation, const OutputPatch<DomainInputType, NUM_CONTROL_POINTS> patch)
{
	PixelInputType Output;

	//Output.position = float4(patch[0].position * domain.x + patch[1].position * domain.y + patch[2].position * domain.z, 1);
	float3 vertexPosition = patch[0].position * domain.x + patch[1].position * domain.y + patch[2].position * domain.z;
	//float3 vertexPosition = domain.x * patch[0].position + domain.y * patch[1].position + domain.z * patch[2].position;

	//Output.position = float4(vertexPosition, 1.0f);

	Output.position = mul(float4(vertexPosition, 1.0f), cWorld);
	Output.position = mul(Output.position, cView);
	Output.position = mul(Output.position, cProjection);

	Output.color = patch[0].color;

	return Output;
}
