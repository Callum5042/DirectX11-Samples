#include "ShaderData.hlsli"

// Entry point for the vertex shader - will be executed for each pixel
float4 main(PixelInput input) : SV_TARGET
{
	return float4(0.6f, 0.6f, 0.6f, 1.0f);
}