// Vertex input
struct VertexInput
{
	float3 position : POSITION;
	float3 normal : NORMAL;
};

// Vertex output / pixel input structure
struct VertexOutput
{
	float4 positionClipSpace : SV_POSITION;
	float3 position : POSITION;
	float3 normal : NORMAL;
};

// World constant buffer
cbuffer WorldBuffer : register(b0)
{
	matrix cWorld;
	matrix cView;
	matrix cProjection;
	matrix cWorldInverse;
}

// Directional light value
struct DirectionalLight
{
	float4 diffuse;
	float4 ambient;
	float4 specular;
	//float4 direction;

	// Camera is used for calculating the specular value
	float3 cameraPosition;
	float padding;
};

// Light buffer
cbuffer LightBuffer : register(b1)
{
	DirectionalLight cDirectionalLight;
}