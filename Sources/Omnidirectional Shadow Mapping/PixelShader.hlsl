#include "ShaderData.hlsli"

float CalculateShadowFactor(PixelInput input)
{
	// Calculate distance to light
	float3 depthToLightVector = input.position - cLightPointPosition.xyz;
	float depthToLight = length(depthToLightVector);

	// Sample
	float far_plane = 100.0f;

	/*float compare = (depthToLight / far_plane);
	float shadowDepth1 = gShadowMapTexture.SampleCmpLevelZero(gShadowSampler, depthToLightVector / far_plane, compare).r;
	return shadowDepth1;*/

	float shadowDepth = gShadowMapTexture.Sample(gShadowSampler1, depthToLightVector / far_plane).r;

	float bias = 0.0005f;
	if (shadowDepth < (depthToLight / far_plane) - bias)
	{
		return 0.0f;
	}
	else
	{
		return 1.0f;
	}
}

float4 CalculatePointLighting(PixelInput input)
{
	float4 light_colour = float4(0.6f, 0.6f, 0.6f, 1.0f);

	// Shadow map?
	float shadow_factor = CalculateShadowFactor(input);

	// Diffuse lighting
	float3 light_vector = normalize(cLightPointPosition.xyz - input.position);
	float4 diffuse_light = saturate(dot(light_vector, input.normal)) * light_colour * shadow_factor;

	// Ambient lighting
	float4 ambient_light = light_colour * 0.2f;

	// Specular lighting
	float3 view_direction = normalize(cCameraPosition.xyz - input.position);
	float3 reflect_direction = reflect(-light_vector, input.normal);
	float spec = pow(max(dot(view_direction, reflect_direction), 0.0), 8.0f);
	float4 specular_light = float4(spec * light_colour.xyz * 0.2f, 1.0f) * shadow_factor;

	return diffuse_light + ambient_light + specular_light;
}

// Entry point for the vertex shader - will be executed for each pixel
float4 main(PixelInput input) : SV_TARGET
{
	// Interpolating normal can unnormalize it, so normalize it.
	input.normal = normalize(input.normal);

	// Calculate directional light
	float4 light_colour = CalculatePointLighting(input);

	return light_colour;
}