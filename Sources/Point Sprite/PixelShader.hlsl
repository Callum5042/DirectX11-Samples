#include "ShaderData.hlsli"

// Entry point for the vertex shader - will be executed for each pixel
float4 main(PixelInput input) : SV_TARGET
{
	//float4 diffuse_texture = gTextureDiffuse.Sample(gSamplerAnisotropic, input.tex);
	return float4(1.0f, 0.0f, 0.0f, 1.0f);
}