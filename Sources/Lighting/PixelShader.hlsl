#include "ShaderData.hlsli"

float4 CalculateDirectionalLighting(float3 position, float3 normal)
{
	// Light direction
	float3 light_direction = float3(1.0f, -0.7f, 0.4f);

	// Diffuse lighting
	float4 diffuse_light = saturate(dot(-light_direction, normal)) * cDirectionalLight.diffuse;

	// Ambient lighting
	float4 ambient_light = cDirectionalLight.ambient;

	// Specular lighting
	float3 viewDirection = normalize(cDirectionalLight.cameraPosition - position);
	float3 reflectDirection = reflect(light_direction, normal);

	float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), cDirectionalLight.specular.w);
	float spec_strength = 0.5;
	float4 specular_light = float4(spec * cDirectionalLight.specular.xyz * spec_strength, 1.0f);

	// Combine the lights
	return diffuse_light + ambient_light + specular_light;
}

// Entry point for the vertex shader - will be executed for each pixel
float4 main(VertexOutput input) : SV_TARGET
{
	// Interpolating normal can unnormalize it, so normalize it.
	input.normal = normalize(input.normal);

	// Calculate directional light
	float4 light_colour = CalculateDirectionalLighting(input.position.xyz, input.normal);

	return light_colour;
}