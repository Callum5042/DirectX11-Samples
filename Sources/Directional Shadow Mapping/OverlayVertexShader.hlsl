#include "OverlayShaderData.hlsli"

PixelInput main(VertexInput input)
{
    PixelInput output;
    output.position = input.position;
    output.positionClipSpace = float4(input.position, 1.0f);
	
    return output;
}