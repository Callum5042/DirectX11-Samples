#include "ShaderData.hlsli"

// Entry point for the vertex shader - will be executed for each pixel
float4 main(PixelInputType input) : SV_TARGET
{
	return float4(0.0f, 0.0f, 0.0f, 1.0f);
}