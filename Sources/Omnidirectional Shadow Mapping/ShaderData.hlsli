// Vertex input
struct VertexInput
{
	float3 position : POSITION;
	float3 normal : NORMAL;
};

// Vertex output / pixel input structure
struct PixelInput
{
	float4 positionClipSpace : SV_POSITION;
	float3 position : POSITION;
	float3 normal : NORMAL;
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

// Point light buffer
cbuffer PointLightBuffer : register(b2)
{
	float4 cLightPointPosition;
}

// Shadow map
TextureCube gShadowMapTexture : register(t0);
SamplerState gSamplerAnisotropic : register(s0);