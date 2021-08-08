#include "ShaderData.hlsli"

// Entry point for the vertex shader - will be executed for each pixel
float4 main(PixelInput input) : SV_TARGET
{
	// Sample textures
	float4 grass_texture = gTetureGrass.Sample(gSamplerAnisotropic, input.tex_tiled);
	float4 brick_texture = gTextureBrick.Sample(gSamplerAnisotropic, input.tex_tiled);
	float4 alpha_texture = gTexturAlpha.Sample(gSamplerAnisotropic, input.tex);

	// Linear interpolation of the 3 textures
	float4 final_colour = lerp(grass_texture, brick_texture, alpha_texture);

	return final_colour;
}