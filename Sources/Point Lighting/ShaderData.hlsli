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
cbuffer PointLightBuffer : register(b2)
{
	float4 cLightPointPosition;
}

// Directional light value
//struct DirectionalLight
//{
//	float4 diffuse;
//	float4 ambient;
//	float4 specular;
//	//float4 direction;
//
//	// Camera is used for calculating the specular value
//	float3 cameraPosition;
//	float padding;
//};
//
//// Light buffer
//cbuffer LightBuffer : register(b2)
//{
//	DirectionalLight cDirectionalLight;
//}