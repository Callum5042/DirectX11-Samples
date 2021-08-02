#include "ShaderData.hlsli"

[maxvertexcount(4)]
void main(point GeometryInput input[1], inout TriangleStream<PixelInput> output)
{
	float width = input[0].size.x;
	float height = input[0].size.y;

	float3 vertices[4];
	vertices[0] = float3(-width, +height, 0.0f);
	vertices[1] = float3(+width, +height, 0.0f);
	vertices[2] = float3(-width, -height, 0.0f);
	vertices[3] = float3(+width, -height, 0.0f);

	[unroll]
	for (uint i = 0; i < 4; i++)
	{
		PixelInput element;

		element.position = mul(float4(vertices[i], 1.0f), cWorld);
		element.position = mul(element.position, cView);
		element.position = mul(element.position, cProjection);

		output.Append(element);
	}
}