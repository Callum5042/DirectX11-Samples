#include "ShaderData.hlsli"

// Entry point for the vertex shader - will be executed for each pixel
float4 main(PixelInput input) : SV_TARGET
{
	float3 light_vector = normalize(cLightPointPosition.xyz - input.position);
	float lightVecLength = length(light_vector);



	return cLightPointPosition / 25.0f;
}