// Vertex input
struct VertexInput
{
	float3 position : POSITION;
	float2 tex : TEXCOORD;
};

// Vertex output / pixel input structure
struct PixelInput
{
	float4 positionClipSpace : SV_POSITION;
	float3 position : POSITION;
	float2 tex : TEXCOORD;
};

// Camera buffer
cbuffer CameraBuffer : register(b0)
{
	matrix cView;
	matrix cProjection;
	float4 cCameraPosition;
}

// World buffer
cbuffer WorldBuffer : register(b1)
{
	matrix cWorld;
	matrix cWorldInverse;
}

// Texture sampler
SamplerState gSamplerAnisotropic : register(s0);

// Textures
Texture2D gTextureDiffuse : register(t0);