#include "ShaderData.hlsli"

[domain("quad")]
PixelInputType main(HullConstDataOutput input, float2 uv : SV_DomainLocation, const OutputPatch<DomainInputType, 4> quad)
{
	PixelInputType Output;

	// Spatial position
	float3 v1 = lerp(quad[0].position, quad[1].position, uv.x);
	float3 v2 = lerp(quad[2].position, quad[3].position, uv.x);
	float3 vertexPosition = lerp(v1, v2, uv.y);

	// UV
	float4 t1 = lerp(quad[0].color, quad[1].color, uv.x);
	float4 t2 = lerp(quad[2].color, quad[3].color, uv.x);
	float4 colour = lerp(t1, t2, uv.y);

	// Update position
	Output.position = mul(float4(vertexPosition, 1.0f), cWorld);
	Output.position = mul(Output.position, cView);
	Output.position = mul(Output.position, cProjection);

	// Update colour
	Output.color = colour;

	return Output;
}
