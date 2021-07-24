#include "ShaderData.hlsli"

[domain("tri")]
PixelInputType main(HullConstDataOutput input, float3 domain : SV_DomainLocation, const OutputPatch<DomainInputType, 3> patch)
{
	PixelInputType Output;

	float3 vertexPosition = patch[0].position * domain.x + patch[1].position * domain.y + patch[2].position * domain.z;

	// Update position
	Output.position = mul(float4(vertexPosition, 1.0f), cWorld);
	Output.position = mul(Output.position, cView);
	Output.position = mul(Output.position, cProjection);

	// Update colour
	Output.color = patch[0].color * domain.x + patch[1].color * domain.y + patch[2].color * domain.z;

	return Output;
}
