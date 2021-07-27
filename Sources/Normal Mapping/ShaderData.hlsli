// Vertex input
struct VertexInput
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 tex : TEXTURE;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
};

// Vertex output / pixel input structure
struct VertexOutput
{
	float4 positionClipSpace : SV_POSITION;
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 tex : TEXTURE;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
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

	// Camera is used for calculating the specular value
	float3 cameraPosition;
	float padding;
};

// Light buffer
cbuffer LightBuffer : register(b1)
{
	DirectionalLight cDirectionalLight;
}

// Texture sampler
SamplerState gSamplerAnisotropic : register(s0);

// Diffuse texture
Texture2D gTextureDiffuse : register(t0);

// Normal texture
Texture2D gTextureNormal : register(t1);