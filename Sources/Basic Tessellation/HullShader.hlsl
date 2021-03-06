#include "ShaderData.hlsli"

// Patch Constant Function
HullConstDataOutput CalcHSPatchConstants(InputPatch<HullInputType, 3> patch, uint PatchID : SV_PrimitiveID)
{
	HullConstDataOutput pt;

	float tess = cTess.x;

	pt.EdgeTess[0] = tess;
	pt.EdgeTess[1] = tess;
	pt.EdgeTess[2] = tess;

	pt.InsideTess = tess;

	return pt;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("CalcHSPatchConstants")]
DomainInputType main(InputPatch<HullInputType, 3> patch, uint i : SV_OutputControlPointID, uint PatchID : SV_PrimitiveID)
{
	DomainInputType Output;

	// Insert code to compute Output here
	Output.position = patch[i].position;
	Output.color = patch[i].color;

	return Output;
}
