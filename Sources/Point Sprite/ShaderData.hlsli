// Vertex input
struct VertexInput
{
	float3 position : POSITION;
	float2 size : SIZE;
};

// Vertex output / Geometry input
struct GeometryInput
{
	float3 position : POSITION;
	float2 size : SIZE;
};

// Geometry output / Pixel input structure
struct PixelInput
{
	float4 position : SV_POSITION;
};

// World constant buffer
cbuffer WorldBuffer : register(b0)
{
	matrix cWorld;
	matrix cView;
	matrix cProjection;
}

// Texture sampler
SamplerState gSamplerAnisotropic : register(s0);

// Textures
Texture2D gTextureDiffuse : register(t0);