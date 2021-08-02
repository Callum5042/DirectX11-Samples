#include "ShaderData.hlsli"

// Entry point for the vertex shader - will be executed for each vertex
GeometryInput main(VertexInput input)
{
	GeometryInput output;

	// Pass through the vertex shader
	output.position = input.position;

	// Transform to homogeneous clip space.
	//output.position = mul(float4(input.position, 1.0f), cWorld);
	//output.position = mul(output.position, cView);
	//output.position = mul(output.position, cProjection);

	//// Set the vertex colour
	//output.tex = input.tex;



	return output;
}