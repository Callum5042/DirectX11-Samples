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
struct HullConstDataOutput
{
	float EdgeTessFactor[3]			: SV_TessFactor; // e.g. would be [4] for a quad domain
	float InsideTessFactor			: SV_InsideTessFactor; // e.g. would be Inside[2] for a quad domain
};

cbuffer WorldBuffer : register(b0)
{
	matrix cWorld;
	matrix cView;
	matrix cProjection;
	float4 cTess;
}

[domain("tri")]
PixelInputType main(HullConstDataOutput input, float3 domain : SV_DomainLocation, const OutputPatch<DomainInputType, 3> patch)
{
	PixelInputType Output;

	float3 vertexPosition = patch[0].position * domain.x + patch[1].position * domain.y + patch[2].position * domain.z;

	Output.position = mul(float4(vertexPosition, 1.0f), cWorld);
	Output.position = mul(Output.position, cView);
	Output.position = mul(Output.position, cProjection);

	Output.color = patch[0].color * domain.x + patch[1].color * domain.y + patch[2].color * domain.z;

	return Output;
}
