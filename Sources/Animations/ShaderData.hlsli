// Vertex input
struct VertexInput
{
	float3 position : POSITION;
	float4 weight : WEIGHT;
	int4 joint : JOINT;
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
	matrix cBoneTransformation[64];
}

// Camera constant buffer
cbuffer CameraBuffer : register(b1)
{
	matrix cView;
	matrix cProjection;
}