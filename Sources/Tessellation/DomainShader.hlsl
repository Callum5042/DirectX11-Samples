#include "ShaderData.hlsli"

//The new functions
//float3 project(float3 p, float3 c, float3 n)
//{
//	return p - dot(p - c, n) * n;
//}

// Computes the position of a point in the Phong Tessellated triangle
//float3 PhongGeometry(float u, float v, float w, HullConstDataOutput hsc)
//{
//    // Find local space point
//    float3 p = w * hsc.f3B0 + u * hsc.f3B1 + v * hsc.f3B2;
//
//    // Find projected vectors
//    float3 c0 = project(p, hsc.f3B0, hsc.f3N0);
//    float3 c1 = project(p, hsc.f3B1, hsc.f3N1);
//    float3 c2 = project(p, hsc.f3B2, hsc.f3N2);
//
//    // Interpolate
//    float3 q = w * c0 + u * c1 + v * c2;
//    // For blending between tessellated and untessellated model:
//    //float3 r = LERP(p, q, alpha);
//
//    return q;
//}
//
//// Computes the normal of a point in the Phong Tessellated triangle
//float3 PhongNormal(float u, float v, float w, HullConstDataOutput hsc)
//{
//    // Interpolate
//    return normalize(w * hsc.f3N0 + u * hsc.f3N1 + v * hsc.f3N2);
//}

[domain("tri")]
PixelInputType main(HullConstDataOutput input, float3 domain : SV_DomainLocation, const OutputPatch<DomainInputType, 3> patch)
{
	PixelInputType Output;

	float3 vertexPosition = patch[0].position * domain.x + patch[1].position * domain.y + patch[2].position * domain.z;


	Output.position = mul(float4(vertexPosition, 1.0f), cWorld);
	Output.position = mul(Output.position, cView);
	Output.position = mul(Output.position, cProjection);

	//Output.color = quad[0].color;
	Output.color = patch[0].color * domain.x + patch[1].color * domain.y + patch[2].color * domain.z;

	return Output;
}
