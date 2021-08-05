#include "ShaderData.hlsli"

float4 CalculateDirectionalLighting(PixelInput input, float3 bumpedNormal)
{
	float4 diffuse_light_colour = float4(0.6f, 0.6f, 0.6f, 1.0f);
	float4 ambient_light_colour = float4(0.1f, 0.1f, 0.1f, 1.0f);
	float4 specular_light_colour = float4(0.2f, 0.2f, 0.2f, 1.0f);

	// Light direction
	float3 light_direction = cLightDirection.xyz;

	// Ambient lighting
	float4 ambient_light = ambient_light_colour;

	float diffuseFactor = saturate(dot(-light_direction, bumpedNormal));

	[flatten]
	if (diffuseFactor > 0.0f)
	{
		// Diffuse lighting
		float4 diffuse_light = saturate(dot(-light_direction, bumpedNormal)) * diffuse_light_colour;

		// Specular lighting
		float3 view_direction = normalize(cCameraPosition.xyz - input.position.xyz);
		float3 reflect_direction = reflect(light_direction, bumpedNormal);

		float specular_factor = pow(max(dot(view_direction, reflect_direction), 0.0), 32.0f);
		float specular_strength = 5.0f;
		float4 specular_light = float4(specular_factor * specular_light_colour) * specular_strength;

		// Combine the lights
		return diffuse_light + ambient_light + specular_light;
	}
	else
	{
		return ambient_light;
	}
}

float3 CalculateNormalsFromNormalMap(PixelInput input)
{
	float3 normalMapSample = gTextureNormal.Sample(gSamplerAnisotropic, input.tex).rgb;

	// Uncompress each component from [0,1] to [-1,1].
	float3 normalT = normalize(normalMapSample * 2.0f - 1.0f);

	// Build orthonormal basis.
	float3 N = normalize(input.normal); // Normal
	float3 T = normalize(input.tangent - dot(input.tangent, N) * N); // Tangent
	float3 B = cross(N, T); // Bi-Tangent

	float3x3 TBN = float3x3(T, B, N);

	// Transform from tangent space to world space.
	float3 bumpedNormalW = mul(normalT, TBN);

	return normalize(bumpedNormalW);
}

// Entry point for the vertex shader - will be executed for each pixel
float4 main(PixelInput input) : SV_TARGET
{
	// Interpolating normal can unnormalize it, so normalize it.
	input.normal = normalize(input.normal);

	// Calculate normals from sampling the normal map
	//input.normal = CalculateNormalsFromNormalMap(input);
	float3 bumped_normal = CalculateNormalsFromNormalMap(input);

	// Calculate directional light
	float4 light_colour = CalculateDirectionalLighting(input, bumped_normal);

	// Texture colour
	float4 texture_colour = gTextureDiffuse.Sample(gSamplerAnisotropic, input.tex);

	// Combine colours
	float4 colour = light_colour * texture_colour;

	return colour;
}