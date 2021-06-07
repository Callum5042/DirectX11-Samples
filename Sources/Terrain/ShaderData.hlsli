// World constant buffer
cbuffer WorldBuffer : register(b0)
{
	matrix cWorld;
	matrix cView;
	matrix cProjection;
	float4 cTess;
}

// Vertex input
struct VertexInputType
{
	float3 position : POSITION;
	float2 tex : TEXTURE;
};

// Hull input / vertex output
struct HullInputType
{
	float3 position : POSITION;
	float2 tex : TEXTURE;
};

// Domain input / Hull output
struct DomainInputType
{
	float3 position : POSITION;
	float2 tex : TEXTURE;
};

// Pixel input / Domain output
struct PixelInputType
{
    float4 position : SV_POSITION;
	float2 tex : TEXTURE;
};

// Output patch constant data.
struct HullConstDataOutput
{
	float EdgeTess[4] : SV_TessFactor; // e.g. would be [4] for a quad domain
	float InsideTess[2] : SV_InsideTessFactor; // e.g. would be Inside[2] for a quad domain
};

// Texture sampler
SamplerState gSamplerAnisotropic : register(s0);

// Textures
Texture2D gTextureDiffuse : register(t0);