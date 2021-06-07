#include "ShaderData.hlsli"

[domain("quad")]
PixelInputType main(HullConstDataOutput input, float2 uv : SV_DomainLocation, const OutputPatch<DomainInputType, 4> quad)
{
	PixelInputType Output;

	// Spatial position
	float3 v1 = lerp(quad[0].position, quad[1].position, uv.x);
	float3 v2 = lerp(quad[2].position, quad[3].position, uv.x);
	float3 vertexPosition = lerp(v1, v2, uv.y);

	Output.position = mul(float4(vertexPosition, 1.0f), cWorld);
	Output.position = mul(Output.position, cView);
	Output.position = mul(Output.position, cProjection);

	// UV
	float2 t1 = lerp(quad[0].tex, quad[1].tex, uv.x);
	float2 t2 = lerp(quad[2].tex, quad[3].tex, uv.x);
	float2 tex = lerp(t1, t2, uv.y);

	Output.tex = tex;

	return Output;
}
