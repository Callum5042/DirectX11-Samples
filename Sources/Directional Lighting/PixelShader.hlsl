#include "ShaderData.hlsli"

float4 CalculateDirectionalLighting(float3 position, float3 normal)
{
	float4 light_colour = float4(0.6f, 0.6f, 0.6f, 1.0f);

	// Light direction
	float3 light_direction = float3(1.0f, -0.7f, 0.4f);

	// Diffuse lighting
	float4 diffuse_light = saturate(dot(-light_direction, normal)) * light_colour;

	// Ambient lighting
	float4 ambient_light = light_colour * 0.2f;

	// Specular lighting
	/*float3 view_direction = normalize(cCameraPosition.xyz - position);
	float3 reflect_direction = reflect(-light_direction, normal);
	float spec = pow(max(dot(view_direction, reflect_direction), 0.0), 8.0f);
	float4 specular_light = float4(spec * light_colour.xyz * 0.2f, 1.0f);*/

	float3 view_direction = normalize(cCameraPosition.xyz - position);
	float3 reflect_direction = reflect(light_direction, normal);

	float spec = pow(max(dot(view_direction, reflect_direction), 0.0), 8.0f);
	float4 specular_light = float4(spec * light_colour.xyz * 0.2f, 1.0f);

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