// World constant buffer
cbuffer WorldBuffer : register(b0)
{
	matrix cWorld;
	matrix cView;
	matrix cProjection;
}

// Vertex input
struct VertexInput
{
	float3 position : POSITION;
	float4 colour : COLOUR;
};

// Vertex output / hull input
struct VertexOutput
{
	float3 position : POSITION;
	float4 colour : COLOUR;
};

// Hull output / domain input
struct HullOutput
{
	float3 position : POSITION;
	float4 colour : COLOUR;
};

// Domain output / pixel input
struct DomainOutput
{
	float4 position : SV_POSITION;
	float4 colour : COLOUR;
};

struct ConstantOutputType
{
	float edges[3] : SV_TessFactor;
	float inside : SV_InsideTessFactor;
};