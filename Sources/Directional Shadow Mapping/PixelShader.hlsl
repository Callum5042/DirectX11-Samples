#include "ShaderData.hlsli"

static const float SMAP_SIZE = 1024.0f;
static const float SMAP_DX = 1.0f / SMAP_SIZE;

float4 CalculateDirectionalLighting(float3 position, float3 normal, PixelInput input)
{
	float4 diffuse_light_colour = float4(0.6f, 0.6f, 0.6f, 1.0f);
	float4 ambient_light_colour = float4(0.1f, 0.1f, 0.1f, 1.0f);
	float4 specular_light_colour = float4(0.2f, 0.2f, 0.2f, 1.0f);

	// Light direction
	float3 light_direction = cLightDirection.xyz;

	// Shadow map?
	float2 shadowTexCoords;
	shadowTexCoords.x = input.lightViewPosition.x / input.lightViewPosition.w / 2.0f + 0.5f;
	shadowTexCoords.y = -input.lightViewPosition.y / input.lightViewPosition.w / 2.0f + 0.5f;
	float pixelDepth = input.lightViewPosition.z / input.lightViewPosition.w;

	float lighting = 1;

	const float dx = SMAP_DX;
	//float bias = 0.001f;
	//float bias = max(0.05f * (1.0 - dot(-light_direction, normal)), 0.005f);

	// Check if the pixel texture coordinate is in the view frustum of the 
	// light before doing any shadow work.
	if ((saturate(shadowTexCoords.x) == shadowTexCoords.x) && (saturate(shadowTexCoords.y) == shadowTexCoords.y) && (pixelDepth > 0))
	{
		lighting = gShadowMap.SampleCmpLevelZero(gShadowSampler, shadowTexCoords, pixelDepth).r;
	}

	// Diffuse lighting
	float4 diffuse_light = saturate(dot(-light_direction, normal)) * diffuse_light_colour * lighting;

	// Ambient lighting
	float4 ambient_light = ambient_light_colour;

	// Specular lighting
	float3 view_direction = normalize(cCameraPosition.xyz - position);
	float3 reflect_direction = reflect(light_direction, normal);

	float specular_factor = pow(max(dot(view_direction, reflect_direction), 0.0), 16.0f);
	float4 specular_light = float4(specular_factor * specular_light_colour) * lighting;

	// Combine the lights
	return diffuse_light + ambient_light + specular_light;
}

// Entry point for the vertex shader - will be executed for each pixel
float4 main(PixelInput input) : SV_TARGET
{
	// Interpolating normal can unnormalize it, so normalize it.
	input.normal = normalize(input.normal);

	// Calculate directional light
	float4 light_colour = CalculateDirectionalLighting(input.position.xyz, input.normal, input);

	return light_colour;
}