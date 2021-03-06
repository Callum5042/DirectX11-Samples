#include "ShaderData.hlsli"

// Patch Constant Function
HullConstDataOutput CalcHSPatchConstants(InputPatch<HullInputType, 4> patch, uint PatchID : SV_PrimitiveID)
{
	HullConstDataOutput pt;

	float tess = cTess.x;

	pt.EdgeTess[0] = tess;
	pt.EdgeTess[1] = tess;
	pt.EdgeTess[2] = tess;
	pt.EdgeTess[3] = tess;

	pt.InsideTess[0] = tess;
	pt.InsideTess[1] = tess;

	return pt;
}

[domain("quad")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("CalcHSPatchConstants")]
DomainInputType main(InputPatch<HullInputType, 4> patch, uint i : SV_OutputControlPointID, uint PatchID : SV_PrimitiveID)
{
	DomainInputType Output;

	// Insert code to compute Output here
	Output.position = patch[i].position;
	Output.tex = patch[i].tex;

	return Output;
}
