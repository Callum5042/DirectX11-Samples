// World constant buffer
cbuffer WorldBuffer : register(b0)
{
	matrix cWorld;
	matrix cView;
	matrix cProjection;
	float3 cCameraPosition;
	float padding;
}

// Vertex input
struct VertexInputType
{
	float3 position : POSITION;
	float4 color : COLOR;
};

// Hull input / vertex output
struct HullInputType
{
	float3 position : POSITION;
	float4 color : COLOR;
};

// Domain input / Hull output
struct DomainInputType
{
	float3 position : POSITION;
	float4 color : COLOR;
};

// Pixel input / Domain output
struct PixelInputType
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

// Output patch constant data.
struct HullConstDataOutput
{
	float EdgeTess[4] : SV_TessFactor;
	float InsideTess[2] : SV_InsideTessFactor;
};