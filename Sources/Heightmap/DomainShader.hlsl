#include "ShaderData.hlsli"

[domain("quad")]
PixelInputType main(HullConstDataOutput input, float2 uv : SV_DomainLocation, const OutputPatch<DomainInputType, 4> quad)
{
	PixelInputType output;

	// Spatial position
	float3 v1 = lerp(quad[0].position, quad[1].position, uv.x);
	float3 v2 = lerp(quad[2].position, quad[3].position, uv.x);
	float3 vertexPosition = lerp(v1, v2, uv.y);

	// Update position
	output.position = mul(float4(vertexPosition, 1.0f), cWorld);
	output.position = mul(output.position, cView);
	output.position = mul(output.position, cProjection);

	return output;
}
