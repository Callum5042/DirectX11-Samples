#include "ShaderData.hlsli"

// Entry point for the vertex shader - will be executed for each vertex
HullInputType main(VertexInputType input)
{
	HullInputType output;

	// Transform to homogeneous clip space.
	output.position = input.position;

	// Transform Y axis from heightmap


	return output;
}