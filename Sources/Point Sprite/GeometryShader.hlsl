#include "ShaderData.hlsli"

[maxvertexcount(4)]
void main(point GeometryInput input[1], inout TriangleStream<PixelInput> output)
{
	float3 position = input[0].position;
	float width = input[0].size.x;
	float height = input[0].size.y;

	// Create rectangle vertices
	float3 vertices[4];
	vertices[0] = float3(position.x - width, position.y + height, 0.0f);
	vertices[1] = float3(position.x + width, position.y + height, 0.0f);
	vertices[2] = float3(position.x - width, position.y - height, 0.0f);
	vertices[3] = float3(position.x + width, position.y - height, 0.0f);

	// Append output stream with our 4 new rectangle vertices
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