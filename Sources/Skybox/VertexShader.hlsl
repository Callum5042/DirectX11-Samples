#include "ShaderData.hlsli"

// Entry point for the vertex shader - will be executed for each vertex
VertexOutput main(VertexInput input)
{
	VertexOutput output;

	// Transform to homogeneous clip space.
	output.position = mul(float4(input.position, 1.0f), cWorld);
	output.position = mul(output.position, cView);

	// Set z = w so that z/w = 1 (i.e., skydome always on far plane).
	output.position = mul(output.position, cProjection).xyww; 

	// Set the vertex colour
	output.tex = input.tex;

	return output;
}