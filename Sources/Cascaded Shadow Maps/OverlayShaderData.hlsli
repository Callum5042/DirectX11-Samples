// Vertex input
struct VertexInput
{
    float3 position : POSITION;
    float2 tex : TEXTURE;
};

// Vertex output / pixel input structure
struct PixelInput
{
    float4 position : SV_POSITION;
    float2 tex : TEXTURE;
};

// Texture sampler
SamplerState gSamplerAnisotropic : register(s1);

// Textures
Texture2D gTextureDiffuse : register(t0);