#include "ShaderData.hlsli"

// Entry point for the vertex shader - will be executed for each vertex
PixelInput main(VertexInput input)
{
	PixelInput output;

	// Transform to homogeneous clip space.
	output.positionClipSpace = mul(float4(input.position, 1.0f), cWorld);
	output.positionClipSpace = mul(output.positionClipSpace, cView);
	output.positionClipSpace = mul(output.positionClipSpace, cProjection);

	// Transform to world space.
	output.position = mul(float4(input.position, 1.0f), cWorld).xyz;

	// Transform the normals by the inverse world space
	output.normal = mul(input.normal, (float3x3)cWorldInverse).xyz;

	// Transform to homogeneous clip space.
	output.shadowHomoPos = mul(float4(input.position, 1.0f), cWorld);
	output.shadowHomoPos = mul(output.shadowHomoPos, cView);

	// Set z = w so that z/w = 1 (i.e., skydome always on far plane).
	output.shadowHomoPos = mul(output.shadowHomoPos, cProjection).xyww;

	return output;
}