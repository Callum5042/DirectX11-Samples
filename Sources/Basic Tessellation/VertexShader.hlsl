#include "ShaderData.hlsli"

// Vertex input
struct VertexInputType
{
	float3 position : POSITION;
	float4 color : COLOR;
};

struct HullInputType
{
	float3 position : POSITION;
	float4 color : COLOR;
};

// Vertex out / pixel input
//struct PixelInputType
//{
//	float4 position : SV_POSITION;
//	float4 color : COLOUR;
//};

// World constant buffer
cbuffer WorldBuffer : register(b0)
{
	matrix cWorld;
	matrix cView;
	matrix cProjection;
}

// Entry point for the vertex shader - will be executed for each vertex
HullInputType main(VertexInputType input)
{
	HullInputType output;

	// Transform to homogeneous clip space.
	output.position = input.position;
	/*output.position = mul(float4(input.position, 1.0f), cWorld);
	output.position = mul(output.position, cView);
	output.position = mul(output.position, cProjection);*/

	// Set the vertex colour
	output.color = input.color;

	return output;
}