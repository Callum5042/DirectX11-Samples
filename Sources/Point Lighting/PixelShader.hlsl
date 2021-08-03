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
	/*float3 viewDir = normalize(mPointLight.mCameraPos - position);
	float3 reflectDir = reflect(-lightVec, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), mDirectionalLight.mSpecular.w * mMaterial.mSpecular.w);
	float4 specular_light = float4(spec * mPointLight.mSpecular.xyz * mMaterial.mSpecular.xyz, 1.0f);*/

	return diffuse_light + ambient_light;
}

// Entry point for the vertex shader - will be executed for each pixel
float4 main(VertexOutput input) : SV_TARGET
{
	// Interpolating normal can unnormalize it, so normalize it.
	input.normal = normalize(input.normal);

	// Calculate directional light
	float4 light_colour = CalculatePointLighting(input.position, input.normal);

	return light_colour;
}