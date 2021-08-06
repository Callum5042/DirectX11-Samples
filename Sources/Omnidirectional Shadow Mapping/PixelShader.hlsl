#include "ShaderData.hlsli"

float4 CalculatePointLighting(float3 position, float3 normal)
{
	float4 light_colour = float4(0.6f, 0.6f, 0.6f, 1.0f);

	// Diffuse lighting
	float3 light_vector = normalize(cLightPointPosition.xyz - position);
	float4 diffuse_light = saturate(dot(light_vector, normal)) * light_colour;

	// Ambient lighting
	float4 ambient_light = light_colour * 0.2f;

	// Specular lighting
	float3 view_direction = normalize(cCameraPosition.xyz - position);
	float3 reflect_direction = reflect(-light_vector, normal);
	float spec = pow(max(dot(view_direction, reflect_direction), 0.0), 8.0f);
	float4 specular_light = float4(spec * light_colour.xyz * 0.2f, 1.0f);

	return diffuse_light + ambient_light + specular_light;
}

// Entry point for the vertex shader - will be executed for each pixel
float4 main(PixelInput input) : SV_TARGET
{
	// Interpolating normal can unnormalize it, so normalize it.
	input.normal = normalize(input.normal);

	// Calculate directional light
	float4 light_colour = CalculatePointLighting(input.position, input.normal);

	//float4 tex = gShadowMapTexture.Sample(gSamplerAnisotropic, input.position);

	return light_colour;
}