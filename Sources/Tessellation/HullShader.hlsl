#include "ShaderData.hlsli"

//cbuffer TessellationBuffer
//{
//	float tessellationAmount;
//	float3 padding;
//};

// Patch Constant Function
ConstantOutputType ColorPatchConstantFunction(InputPatch<VertexOutput, 3> inputPatch, uint patchId : SV_PrimitiveID)
{
	ConstantOutputType output;

	// Set the tessellation factors for the three edges of the triangle.
	output.edges[0] = 12.0f;
	output.edges[1] = 12.0f;
	output.edges[2] = 12.0f;

	// Set the tessellation factor for tessallating inside the triangle.
	output.inside = 12.0f;

	return output;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("ColorPatchConstantFunction")]
HullOutput main(InputPatch<VertexOutput, 3> patch, uint pointId : SV_OutputControlPointID, uint PatchID : SV_PrimitiveID )
{
	HullOutput output;

	// Set the position for this control point as the output position.
	output.position = patch[pointId].position;

	// Set the input color as the output color.
	output.colour = patch[pointId].colour;

	return output;
}
