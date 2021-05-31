// Input control point
struct HullInputType
{
	float3 position : POSITION;
	float4 color : COLOR;
};

// Output control point
struct DomainInputType
{
	float3 position : POSITION;
	float4 color : COLOR;
};

// Output patch constant data.
struct HullConstDataOutput
{
	float EdgeTess[3] : SV_TessFactor; // e.g. would be [4] for a quad domain
	float InsideTess : SV_InsideTessFactor; // e.g. would be Inside[2] for a quad domain
};

cbuffer WorldBuffer : register(b0)
{
	matrix cWorld;
	matrix cView;
	matrix cProjection;
	float4 cTess;
}

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
