#include "SkyboxShaderData.hlsli"

// Entry point for the vertex shader - will be executed for each vertex
VertexOutput main(VertexInput input)
{
	VertexOutput output;

	// Transform to homogeneous clip space.
	output.positionClipSpace = mul(float4(input.position, 1.0f), cWorld);
	output.positionClipSpace = mul(output.positionClipSpace, cView);

	// Set z = w so that z/w = 1 (i.e., skydome always on far plane).
	output.positionClipSpace = mul(output.positionClipSpace, cProjection).xyww;

	// Use local vertex position as cubemap lookup vector.
	output.position = input.position;

	return output;
}