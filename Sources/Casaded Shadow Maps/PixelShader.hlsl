#include "ShaderData.hlsli"

static const float SMAP_SIZE = 4096.0f;
static const float SMAP_DX = 1.0f / SMAP_SIZE;

int CalculateCascadeLayer(float pixel_depth)
{
	for (int i = 0; i < cCascadeTotal; ++i)
	{
		if (pixel_depth < cCascadePlaneDistance[i].x)
		{
			return i;
		}
	}

	return cCascadeTotal - 1;
}

float CalculateShadowFactor(PixelInput input)
{
	// Calculate the cascade level
	float4 pixel_view_space = mul(float4(input.position, 1.0f), cView);
	float pixel_depth = abs(pixel_view_space.z);
	int layer = CalculateCascadeLayer(pixel_depth);

	// Calculate light view proj matrix based on which cascade we are in
	float4 light_view_projection = float4(input.position, 1.0f);
	light_view_projection = mul(light_view_projection, cLightView[layer]);
	light_view_projection = mul(light_view_projection, cLightProjection[layer]);

	// Calculate pixels depth
	float shadow_depth = light_view_projection.z;
	if (shadow_depth > 1.0f)
	{
		return 0.0f;
	}

	// Calculate shadow texture coordinates
	float2 tex_coords;
	tex_coords = light_view_projection.xy / light_view_projection.w;
	tex_coords.x = +tex_coords.x * 0.5f + 0.5f;
	tex_coords.y = -tex_coords.y * 0.5f + 0.5f;

	// Kernel for soft shadows
	const float dx = SMAP_DX;
	const float2 offsets[9] =
	{
		float2(-dx,  -dx), float2(0.0f,  -dx), float2(dx,  -dx),
		float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
		float2(-dx,  +dx), float2(0.0f,  +dx), float2(dx,  +dx)
	};

	// Sample
	float lighting = 1.0f;

	[unroll]
	for (int j = 0; j < 9; ++j)
	{
		float closest_depth = gShadowMap.SampleCmpLevelZero(gShadowSampler, float3(tex_coords.xy + offsets[j], layer), shadow_depth).r;
		lighting += shadow_depth > closest_depth ? 1.0 : 0.0;
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

	// Shadow map
	float shadow_factor = CalculateShadowFactor(input);

	// Diffuse lighting
	float4 diffuse_light = saturate(dot(-light_direction, normal)) * diffuse_light_colour;

	// Ambient lighting
	float4 ambient_light = ambient_light_colour;

	// Specular lighting
	float3 view_direction = normalize(cCameraPosition.xyz - position);
	float3 reflect_direction = reflect(light_direction, normal);

	float specular_factor = pow(max(dot(view_direction, reflect_direction), 0.0), 16.0f);
	float4 specular_light = float4(specular_factor * specular_light_colour);

	// Combine the lights
	return ambient_light + ((diffuse_light + specular_light) * (1.0f - shadow_factor));
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