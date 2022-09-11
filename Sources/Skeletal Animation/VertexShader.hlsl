#include "ShaderData.hlsli"

// Entry point for the vertex shader - will be executed for each vertex
VertexOutput main(VertexInput input)
{
	VertexOutput output;

	/*matrix skinMatrix =
		input.weight.x * cBoneTransform[int(input.bone.x)] +
		input.weight.y * cBoneTransform[int(input.bone.y)] +
		input.weight.z * cBoneTransform[int(input.bone.z)] +
		input.weight.w * cBoneTransform[int(input.bone.w)];*/

	// Transform to homogeneous clip space.
	output.position = mul(float4(input.position, 1.0f), cWorld);
	//output.position = mul(float4(input.position, 1.0f), skinMatrix);
	//output.position = mul(output.position, cWorld);
	output.position = mul(output.position, cView);
	output.position = mul(output.position, cProjection);

	// Set the vertex colour
	output.colour = input.colour;

	return output;
}