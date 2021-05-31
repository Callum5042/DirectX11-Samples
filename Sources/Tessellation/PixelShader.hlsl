#include "ShaderData.hlsli"

struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

// Entry point for the vertex shader - will be executed for each pixel
float4 main(PixelInputType input) : SV_TARGET
{
    return input.color;
}