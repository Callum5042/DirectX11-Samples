#include "ShaderData.hlsli"

float4 CalculateDirectionalLighting(float3 position, float3 normal)
{
	float4 diffuse_light_colour = float4(0.6f, 0.6f, 0.6f, 1.0f);
	float4 ambient_light_colour = float4(0.4f, 0.4f, 0.4f, 1.0f);
	float4 specular_light_colour = float4(0.6f, 0.6f, 0.6f, 1.0f);

	// Light direction
	float3 light_direction = float3(0.00593046332, -0.489380002, -0.872050464);

	// Diffuse lighting
	float4 diffuse_light = saturate(dot(-light_direction, normal)) * diffuse_light_colour;

	// Ambient lighting
	float4 ambient_light = ambient_light_colour;

	// Specular lighting
	float3 viewDirection = normalize(cDirectionalLight.cameraPosition - position);
	float3 reflectDirection = reflect(light_direction, normal);

	float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), 16.0f);
	float4 specular_light = float4(spec * specular_light_colour.xyz, 1.0f);

	// Combine the lights
	return diffuse_light + ambient_light + specular_light;
}

float3 CalculateNormalsFromNormalMap(float3 normal, float2 tex, float3 tangent)
{
	float3 normalMapSample = gTextureNormal.Sample(gSamplerAnisotropic, tex).rgb;

	// Uncompress each component from [0,1] to [-1,1].
	float3 normalT = normalize(normalMapSample * 2.0f - 1.0f);

	// Build orthonormal basis.
	float3 N = normal;
	float3 T = normalize(tangent - dot(tangent, N) * N);
	float3 B = cross(N, T);

	float3x3 TBN = float3x3(T, B, N);

	// Transform from tangent space to world space.
	float3 bumpedNormalW = mul(normalT, TBN);

	return bumpedNormalW;
}

// Entry point for the vertex shader - will be executed for each pixel
float4 main(VertexOutput input) : SV_TARGET
{
	// Interpolating normal can unnormalize it, so normalize it.
	input.normal = normalize(input.normal);

	// Calculate normals from normal map texture
	input.normal = CalculateNormalsFromNormalMap(input.normal, input.tex, input.tangent);

	// Calculate directional light
	float4 light_colour = CalculateDirectionalLighting(input.position.xyz, input.normal);

	// Texture colour
	float4 texture_colour = gTextureDiffuse.Sample(gSamplerAnisotropic, input.tex);

	// Combine colours
	float4 colour = light_colour * texture_colour;

	return colour;
}