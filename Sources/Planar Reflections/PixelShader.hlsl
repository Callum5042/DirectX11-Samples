#include "ShaderData.hlsli"

// Entry point for the vertex shader - will be executed for each pixel
float4 main(PixelInput input) : SV_TARGET
{
	float4 diffuse_texture = gTextureDiffuse.Sample(gSamplerAnisotropic, input.tex);

	diffuse_texture.a = 0.5;
	return diffuse_texture;
}