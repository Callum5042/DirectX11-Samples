#include "ShaderData.hlsli"

// Entry point for the vertex shader - will be executed for each pixel
float4 main(PixelInputType input) : SV_TARGET
{
	return input.color;
}