// Vertex input
struct VertexInput
{
	float3 position : POSITION;
	float4 colour : COLOUR;
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
