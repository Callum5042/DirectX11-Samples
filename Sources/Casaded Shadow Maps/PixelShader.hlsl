#include "ShaderData.hlsli"

static const float SMAP_SIZE = 4096.0f;
static const float SMAP_DX = 1.0f / SMAP_SIZE;

float CalculateShadowFactor(PixelInput input)
{
	// Calculate shadow texture coordinates
	float2 tex_coords;
	tex_coords.x = +input.lightViewProjection.x / input.lightViewProjection.w / 2.0f + 0.5f;
	tex_coords.y = -input.lightViewProjection.y / input.lightViewProjection.w / 2.0f + 0.5f;

	// Calculate pixels depth
	float pixel_depth = input.lightViewProjection.z / input.lightViewProjection.w;

	// Kernel for soft shadows
	const float dx = SMAP_DX;
	const float2 offsets[9] =
	{
		float2(-dx,  -dx), float2(0.0f,  -dx), float2(dx,  -dx),
		float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
		float2(-dx,  +dx), float2(0.0f,  +dx), float2(dx,  +dx)
	};

	// Sample and average shadow map for shadow factor
	float lighting = 0.0f;

	[unroll]
	for (int i = 0; i < 9; ++i)
	{
        // lighting += gShadowMap.SampleCmpLevelZero(gShadowSampler, float3(tex_coords.xy + offsets[i], 1), pixel_depth).r;
        lighting += gShadowMap.SampleCmpLevelZero(gShadowSampler, tex_coords.xy + offsets[i], pixel_depth).r;
    }

	return lighting / 9;
}

float4 CalculateDirectionalLighting(float3 position, float3 normal, PixelInput input)
{
	float4 diffuse_light_colour = float4(0.6f, 0.6f, 0.6f, 1.0f);
	float4 ambient_light_colour = float4(0.1f, 0.1f, 0.1f, 1.0f);
	float4 specular_light_colour = float4(0.2f, 0.2f, 0.2f, 1.0f);

	// Light direction
	float3 light_direction = cLightDirection.xyz;

	// Shadow map?
	float shadow_factor = CalculateShadowFactor(input);

	// Diffuse lighting
	float4 diffuse_light = saturate(dot(-light_direction, normal)) * diffuse_light_colour * shadow_factor;

	// Ambient lighting
	float4 ambient_light = ambient_light_colour;

	// Specular lighting
	float3 view_direction = normalize(cCameraPosition.xyz - position);
	float3 reflect_direction = reflect(light_direction, normal);

	float specular_factor = pow(max(dot(view_direction, reflect_direction), 0.0), 16.0f);
	float4 specular_light = float4(specular_factor * specular_light_colour) * shadow_factor;

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