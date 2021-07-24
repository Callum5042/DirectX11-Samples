#include "ShaderData.hlsli"

// Directional lighting
//float4 CalculateDirectionalLighting(float3 position, float3 normal)
//{
//	float3 lightPos = float3(-5.0f, -5.0f, 0.0f);
//	float3 lightAt = float3(0.0f, 0.0f, 0.0f);
//
//	float3 dir = float3(1.0f, 0.0f, 0.0f); //lightAt - lightPos;
//
//	// Diffuse lighting
//	float3 lightVec = -dir;// -mDirectionalLight.mDirection.xyz;
//	float4 diffuse_light = saturate(dot(lightVec, normal)) * cDirectionalLight.diffuse * cMaterial.diffuse;
//
//	// Ambient lighting
//	float4 ambient_light = cDirectionalLight.ambient * cMaterial.ambient;
//
//	// Specular lighting
//	/*float3 viewDir = normalize(mDirectionalLight.mCameraPos - position);
//	float3 reflectDir = reflect(-lightVec, normal);
//
//	float spec = pow(max(dot(viewDir, reflectDir), 0.0), mDirectionalLight.mSpecular.w * mMaterial.mSpecular.w);
//	float4 specular_light = float4(spec * mDirectionalLight.mSpecular.xyz * mMaterial.mSpecular.xyz, 1.0f);*/
//
//	// Combine all 3 lights
//	return diffuse_light + ambient_light;// +specular_light;
//}

float4 CalculateDirectionalLighting(float3 position, float3 normal)
{
	/*float3 lightPos = float3(-5.0f, -5.0f, 0.0f);
	float3 lightAt = float3(0.0f, 0.0f, 0.0f);*/

	float3 dir = float3(1.0f, 0.7f, 0.4f); //lightAt - lightPos;

	// Diffuse lighting
	float3 lightVec = -dir;
	float4 diffuse_light = dot(lightVec, normal) * cDirectionalLight.diffuse * 0.2f;

	// Ambient lighting
	float4 ambient_light = cDirectionalLight.ambient * 0.2f;

	// Combine the lights 
	return diffuse_light + ambient_light;
}

// Entry point for the vertex shader - will be executed for each pixel
float4 main(VertexOutput input) : SV_TARGET
{
	float4 colour = CalculateDirectionalLighting(input.position.xyz, input.normal);
	return colour;
}