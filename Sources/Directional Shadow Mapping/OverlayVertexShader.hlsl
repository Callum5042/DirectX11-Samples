#include "OverlayShaderData.hlsli"

PixelInput main(VertexInput input)
{
    PixelInput output;
    output.position = float4(input.position, 1.0f);
    output.tex = input.tex;
	
    return output;
}