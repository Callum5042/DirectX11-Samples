#include "ShaderData.hlsli"

static const float SMAP_SIZE = 4096.0f;
static const float SMAP_DX = 1.0f / SMAP_SIZE;

float CalculateShadowFactor(PixelInput input)
{
	float4 fragPosViewSpace = mul(float4(input.position, 1.0f), cView);
	float depthValue = abs(fragPosViewSpace.z);

	float cascadePlaneDistances[3] = { 20.0f, 50.0f, 100.0f };

	int layer = -1;
	int cascadeCount = 3;
	for (int i = 0; i < cascadeCount; ++i)
	{
		if (depthValue < cascadePlaneDistances[i])
		{
			layer = i;
			break;
		}
	}

	if (layer == -1)
	{
		layer = cascadeCount - 1;
	}

	float4 lightViewProjection = float4(input.position, 1.0f);
	lightViewProjection = mul(lightViewProjection, cLightView[layer]);
	lightViewProjection = mul(lightViewProjection, cLightProjection[layer]);

	// Calculate pixels depth
	float pixel_depth = lightViewProjection.z;
	if (pixel_depth > 1.0f)
	{
		return 0.0f;
	}

	// Calculate shadow texture coordinates
	float2 tex_coords;
	tex_coords = lightViewProjection.xy / lightViewProjection.w;
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
	float closestDepth = -1.0f;
	if (layer == 0)
	{
		/*float lighting = 1.0f;

		[unroll]
		for (int i = 0; i < 9; ++i)
		{
			float closestDepth = gShadowMapC1.SampleCmpLevelZero(gShadowSampler, tex_coords.xy + offsets[i], pixel_depth).r;
			lighting += pixel_depth > closestDepth ? 1.0 : 0.0;
		}

		return lighting / 9;*/

		closestDepth = gShadowMapC1.SampleCmpLevelZero(gShadowSampler, tex_coords.xy, pixel_depth).r;
	}
	else if (layer == 1)
	{
		closestDepth = gShadowMapC2.SampleCmpLevelZero(gShadowSampler, tex_coords.xy, pixel_depth).r;
	}
	else if (layer == 2)
	{
		closestDepth = gShadowMapC3.SampleCmpLevelZero(gShadowSampler, tex_coords.xy, pixel_depth).r;
	}

	float shadow = pixel_depth > closestDepth ? 1.0 : 0.0;
	return shadow;
}

float3 VisualizeCascades(PixelInput input)
{
	float4 fragPosViewSpace = mul(float4(input.position, 1.0f), cView);
	float depthValue = abs(fragPosViewSpace.z);

	float cascadePlaneDistances[3] = { 20.0f, 50.0f, 100.0f };

	int layer = -1;
	int cascadeCount = 3;
	for (int i = 0; i < cascadeCount; ++i)
	{
		if (depthValue < cascadePlaneDistances[i])
		{
			layer = i;
			break;
		}
	}

	if (layer == -1)
	{
		layer = cascadeCount;
	}

	float4 lightViewProjection = float4(input.position, 1.0f);
	lightViewProjection = mul(lightViewProjection, cLightView[layer]);
	lightViewProjection = mul(lightViewProjection, cLightProjection[layer]);

	// Calculate pixels depth
	float pixel_depth = lightViewProjection.z;
	if (pixel_depth > 1.0f)
	{
		return 0.0f;
	}

	// Calculate shadow texture coordinates
	float2 tex_coords;
	tex_coords = lightViewProjection.xy / lightViewProjection.w;
	tex_coords.x = +tex_coords.x * 0.5f + 0.5f;
	tex_coords.y = -tex_coords.y * 0.5f + 0.5f;

	float closestDepth = -1.0f;
	if (layer == 0)
	{
		return float3(1.0f, 0.0f, 0.0f);
	}
	else if (layer == 1)
	{
		return float3(0.0f, 1.0f, 0.0f);
	}
	else if (layer == 2)
	{
		return float3(0.0f, 0.0f, 1.0f);
	}

	return float3(0.0f, 0.0f, 0.0f);
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
	// return ambient_light + float4(shadow_factor, 1.0f);
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