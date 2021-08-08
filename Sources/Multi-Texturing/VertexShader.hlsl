#include "ShaderData.hlsli"

// Entry point for the vertex shader - will be executed for each vertex
PixelInput main(VertexInput input)
{
	PixelInput output;

	// Transform to homogeneous clip space.
	output.position = mul(float4(input.position, 1.0f), cWorld);
	output.position = mul(output.position, cView);
	output.position = mul(output.position, cProjection);

	// Transform the texture coordinates (UV's)
	output.tex_tiled = mul(float4(input.tex, 1.0f, 1.0f), cTextureTransform).xy;
	output.tex = input.tex;

	return output;
}