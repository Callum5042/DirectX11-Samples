#include "ShaderData.hlsli"

// Entry point for the vertex shader - will be executed for each vertex
VertexOutput main(VertexInput input)
{
	VertexOutput output;

	// Pass our vertex through without any modifications
	output.position = float4(input.position, 1.0f);

	// Set the vertex colour
	output.colour = input.colour;

	return output;
}
