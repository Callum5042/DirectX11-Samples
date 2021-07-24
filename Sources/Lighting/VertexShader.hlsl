#include "ShaderData.hlsli"

// Entry point for the vertex shader - will be executed for each vertex
VertexOutput main(VertexInput input)
{
	VertexOutput output;

	// Transform to homogeneous clip space.
	output.position = mul(float4(input.position, 1.0f), cWorld);
	output.position = mul(output.position, cView);
	output.position = mul(output.position, cProjection);

	// Pass the normals to pixel shader
	output.normal = input.normal;
	
	// Will want to transform the normals by the world space
	//output.normal = mul(input.normal, (float3x3)InverseWorld).xyz;

	return output;
}