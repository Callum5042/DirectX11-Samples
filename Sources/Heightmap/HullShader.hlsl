#include "ShaderData.hlsli"

float CalcTessFactor(float3 p)
{
	float d = distance(p, cCameraPosition);

	float gMinDist = 40.0f;
	float gMaxDist = 80.0f;

	float gMinTess = 0.0f;
	float gMaxTess = 4.0f;

	// max norm in xz plane (useful to see detail levels from a bird's eye).
	// d = max(abs(p.x - cCameraPosition.x), abs(p.z - cCameraPosition.z));
	float s = saturate((d - gMinDist) / (gMaxDist - gMinDist));

	return pow(2, (lerp(gMaxTess, gMinTess, s)));
}

// Patch Constant Function
HullConstDataOutput CalcHSPatchConstants(InputPatch<HullInputType, 4> patch, uint PatchID : SV_PrimitiveID)
{
	HullConstDataOutput pt;

	float3 e0 = 0.5f * (patch[0].position + patch[2].position);
	float3 e1 = 0.5f * (patch[0].position + patch[1].position);
	float3 e2 = 0.5f * (patch[1].position + patch[3].position);
	float3 e3 = 0.5f * (patch[2].position + patch[3].position);
	float3 c = 0.25f * (patch[0].position + patch[1].position + patch[2].position + patch[3].position);

	pt.EdgeTess[0] = CalcTessFactor(e0);
	pt.EdgeTess[1] = CalcTessFactor(e1);
	pt.EdgeTess[2] = CalcTessFactor(e2);
	pt.EdgeTess[3] = CalcTessFactor(e3);

	pt.InsideTess[0] = CalcTessFactor(c);
	pt.InsideTess[1] = pt.InsideTess[0];

	return pt;
}

[domain("quad")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("CalcHSPatchConstants")]
[maxtessfactor(64.0f)]
DomainInputType main(InputPatch<HullInputType, 4> patch, uint i : SV_OutputControlPointID, uint PatchID : SV_PrimitiveID)
{
	DomainInputType Output;

	// Insert code to compute Output here
	Output.position = patch[i].position;

	return Output;
}
