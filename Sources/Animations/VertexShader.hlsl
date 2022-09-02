#include "ShaderData.hlsli"

// Entry point for the vertex shader - will be executed for each vertex
VertexOutput main(VertexInput input)
{
	VertexOutput output;

	// Skin
	matrix skinMatrix =
		input.weight.x * cBoneTransformation[int(input.joint.x)] +
		input.weight.y * cBoneTransformation[int(input.joint.y)] +
		input.weight.z * cBoneTransformation[int(input.joint.z)] +
		input.weight.w * cBoneTransformation[int(input.joint.w)];

	// Transform to homogeneous clip space.
	output.position = mul(float4(input.position, 1.0f), skinMatrix);
	output.position = mul(output.position, cWorld);
	output.position = mul(output.position, cView);
	output.position = mul(output.position, cProjection);

	return output;
}