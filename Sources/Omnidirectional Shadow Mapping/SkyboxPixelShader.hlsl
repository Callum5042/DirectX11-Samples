#include "SkyboxShaderData.hlsli"

// Entry point for the vertex shader - will be executed for each pixel
float4 main(VertexOutput input) : SV_TARGET
{
	float3 diffuse_texture = gTextureSkybox.Sample(gSamplerAnisotropic, input.position).rrr;
	return float4(diffuse_texture, 1.0f);
}