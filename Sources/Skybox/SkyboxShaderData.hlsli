// Vertex input
struct VertexInput
{
	float3 position : POSITION;
};

// Vertex output / pixel input structure
struct VertexOutput
{
	float4 positionClipSpace : SV_POSITION;
	float3 position : POSITION;
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
TextureCube gTextureSkybox : register(t0);