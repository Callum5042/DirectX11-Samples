// Vertex input
struct VertexInput
{
	float3 position : POSITION;
	float4 colour : COLOUR;
	float4 Weight : WEIGHT;
	int4 Bone : BONE;
};

// Vertex output / pixel input structure
struct VertexOutput
{
	float4 position : SV_POSITION;
	float4 colour : COLOUR;
};

// World constant buffer
cbuffer WorldBuffer : register(b0)
{
	matrix cWorld;
	matrix cView;
	matrix cProjection;
}

// Bone constant buffer
cbuffer BoneBuffer : register(b1)
{
	matrix cBoneOffset[2];
}