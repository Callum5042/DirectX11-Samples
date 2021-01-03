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
