#include "ShaderData.hlsli"

float CalculateShadowFactor(PixelInput input)
{
	float far_plane = 100.0f;

	float3 fragToLight = input.position - cLightPointPosition.xyz;

	// now get current linear depth as the length between the fragment and light position
	float currentDepth = length(fragToLight);

	// ise the fragment to light vector to sample from the depth map    
	float closestDepth = gShadowMapTexture.Sample(gShadowSampler1, normalize(fragToLight)).r;
	//float closestDepth = gShadowMapTexture.SampleCmpLevelZero(gShadowSampler, fragToLight, currentDepth / far_plane).r;

	// it is currently in linear range between [0,1], let's re-transform it back to original depth value
	closestDepth *= far_plane;

	

	// test for shadows
	float bias = 0.05; // we use a much larger bias since depth is now in [near_plane, far_plane] range

	float shadow = currentDepth - bias > closestDepth ? 0.0 : 1.0;
	return shadow + 0.5f;
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