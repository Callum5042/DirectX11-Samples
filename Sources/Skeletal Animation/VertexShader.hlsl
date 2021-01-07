#include "ShaderData.hlsli"

// Entry point for the vertex shader - will be executed for each vertex
VertexOutput main(VertexInput input)
{
	VertexOutput output;

	float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	weights[0] = input.weight.x;
	weights[1] = input.weight.y;
	weights[2] = input.weight.z;
	weights[3] = input.weight.w; //1.0f - weights[0] - weights[1] - weights[2];

	float3 position = float3(0, 0, 0);
	for (int i = 0; i < 4; i++)
	{
		float weight = weights[i];
		int bone_index = input.bone[i];
		matrix transform = cBoneTransform[bone_index];

		position += weight * mul(float4(input.position, 1.0f), transform).xyz;
	}

	// Transform to homogeneous clip space.
	output.position = mul(float4(position, 1.0f), cWorld);
	output.position = mul(output.position, cView);
	output.position = mul(output.position, cProjection);

	// Set the vertex colour
	output.colour = input.colour;

	return output;
}