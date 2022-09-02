// Vertex input
struct VertexInput
{
	float3 position : POSITION;
};

// Vertex output / pixel input structure
struct VertexOutput
{
	float4 position : SV_POSITION;
};

// World constant buffer
cbuffer WorldBuffer : register(b0)
{
	matrix cWorld;
}

// Camera constant buffer
cbuffer CameraBuffer : register(b1)
{
	matrix cView;
	matrix cProjection;
}