#include "ShaderData.hlsli"

static const float zf = 100.0f;
static const float zn = 0.5f;
static const float c1 = zf / (zf - zn);
static const float c0 = -zn * zf / (zf - zn);

float CalculateShadowFactor(PixelInput input)
{
	// Calculate shadow texture coordinates
	//float2 tex_coords;
	//tex_coords.x = +input.lightViewProjection.x / input.lightViewProjection.w / 2.0f + 0.5f;
	//tex_coords.y = -input.lightViewProjection.y / input.lightViewProjection.w / 2.0f + 0.5f;

	//// Calculate pixels depth
	//float pixel_depth = input.lightViewProjection.z / input.lightViewProjection.w;

	//// Kernel for soft shadows
	//const float dx = SMAP_DX;
	//const float2 offsets[9] =
	//{
	//	float2(-dx,  -dx), float2(0.0f,  -dx), float2(dx,  -dx),
	//	float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
	//	float2(-dx,  +dx), float2(0.0f,  +dx), float2(dx,  +dx)
	//};

	//// Sample and average shadow map for shadow factor
	//float lighting = 1.0f;

	//[unroll]
	//for (int i = 0; i < 9; ++i)
	//{
	//	lighting += gShadowMap.SampleCmpLevelZero(gShadowSampler, tex_coords.xy + offsets[i], pixel_depth).r;
	//}

	//return lighting / 9;

	float3 fragToLight = input.position - cLightPointPosition.xyz;

	float closestDepth = gShadowMapTexture.Sample(gShadowSampler1, fragToLight).r;

	float currentDepth = length(fragToLight);

	float bias = 0.05;
	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
	return shadow;
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