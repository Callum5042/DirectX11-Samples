#include "ShaderData.hlsli"

[domain("tri")]
DomainOutput main(ConstantOutputType input, float3 uvwCoord : SV_DomainLocation, const OutputPatch<HullOutput, 3> patch)
{
	DomainOutput output;

	// Determine the position of the new vertex.
	float3 vertexPosition = uvwCoord.x * patch[0].position + uvwCoord.y * patch[1].position + uvwCoord.z * patch[2].position;

	// Calculate the position of the new vertex against the world, view, and projection matrices.
	output.position = mul(float4(vertexPosition, 1.0f), cWorld);
	output.position = mul(output.position, cView);
	output.position = mul(output.position, cProjection);

	// Send the input color into the pixel shader.
	output.colour = patch[0].colour;

	return output;
}
