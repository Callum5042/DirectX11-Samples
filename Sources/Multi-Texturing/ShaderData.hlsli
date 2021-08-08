// Vertex input
struct VertexInput
{
	float3 position : POSITION;
	float2 tex : TEXTURE;
};

// Vertex output / pixel input structure
struct VertexOutput
{
	float4 position : SV_POSITION;
	float2 tex : TEXTURE;
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
Texture2D gTetureGrass : register(t0);
Texture2D gTextureBrick : register(t1);
Texture2D gTexturAlpha : register(t2);