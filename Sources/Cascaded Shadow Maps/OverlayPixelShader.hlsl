#include "OverlayShaderData.hlsli"

float4 main(PixelInput input) : SV_TARGET
{
    float4 diffuse_texture = gTextureDiffuse.Sample(gSamplerAnisotropic, input.tex);
    return diffuse_texture.rrrr;
}