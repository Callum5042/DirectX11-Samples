// Vertex input
struct VertexInput
{
	float3 position : POSITION;
	float2 tex : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
};

// Vertex output / pixel input structure
struct PixelInput
{
	float4 positionClipSpace : SV_POSITION;
	float3 position : POSITION;
	float2 tex : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
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
	matrix cTextureMatrix;
}

// Point light buffer
cbuffer DirectionalLightBuffer : register(b2)
{
	float4 cLightDirection;
}

// Textures
Texture2D gTextureDiffuse : register(t0);
Texture2D gTextureNormal : register(t1);

// Texture sampler - Not bound
SamplerState gSamplerAnisotropic : register(s0);