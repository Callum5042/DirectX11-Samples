#include "SkyboxShaderData.hlsli"

// Entry point for the vertex shader - will be executed for each pixel
float4 main(VertexOutput input) : SV_TARGET
{
	float4 diffuse_texture = gTextureDiffuse.Sample(gSamplerAnisotropic, input.tex);
	return diffuse_texture;
}