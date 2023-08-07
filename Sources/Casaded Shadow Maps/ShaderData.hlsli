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
cbuffer DirectionalLightBuffer : register(b2)
{
	matrix cLightView[3];
    matrix cLightProjection[3];
	float4 cLightDirection;
    int cCascadeTotal;
    float3 padding1;
    float4 cCascadePlaneDistance[3];
}

// Shadow map
Texture2DArray gShadowMap : register(t0);
SamplerComparisonState gShadowSampler : register(s0);