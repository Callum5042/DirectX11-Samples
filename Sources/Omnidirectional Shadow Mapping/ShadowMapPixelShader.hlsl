#include "ShaderData.hlsli"

// Entry point for the vertex shader - will be executed for each pixel
float main(PixelInput input) : SV_DEPTH
{
	float3 light_vector = input.position - cLightPointPosition.xyz;
	float lightVecLength = length(light_vector);

	float far_plane = 100.0f;

	return lightVecLength / far_plane;
}