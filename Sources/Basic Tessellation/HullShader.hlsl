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
struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTess[3] : SV_TessFactor; // e.g. would be [4] for a quad domain
	float InsideTess : SV_InsideTessFactor; // e.g. would be Inside[2] for a quad domain
};

#define NUM_CONTROL_POINTS 3

cbuffer WorldBuffer : register(b0)
{
	matrix cWorld;
	matrix cView;
	matrix cProjection;
	float4 cTess;
}

// Patch Constant Function
HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(InputPatch<HullInputType, NUM_CONTROL_POINTS> patch, uint PatchID : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT pt;

	// Insert code to compute Output here
	/*Output.EdgeTessFactor[0] = 
		Output.EdgeTessFactor[1] = 
		Output.EdgeTessFactor[2] = 
		Output.InsideTessFactor = 15; */// e.g. could calculate dynamic tessellation factors instead

	//float tess = 15;

	/*float3 centerL = 0.25f * (patch[0].position + patch[1].position + patch[2].position + patch[3].position);
	float3 centerW = mul(float4(centerL, 1.0f), cWorld).xyz;

	float d = distance(centerW, gEyePosW);*/

	// Tessellate the patch based on distance from the eye such that
	// the tessellation is 0 if d >= d1 and 60 if d <= d0.  The interval
	// [d0, d1] defines the range we tessellate in.

	/*const float d0 = 20.0f;
	const float d1 = 100.0f;
	float tess = 64.0f * saturate((d1 - d) / (d1 - d0));*/

	float tess = cTess.x;// 10.0f;//cTess.x;

	pt.EdgeTess[0] = tess;
	pt.EdgeTess[1] = tess;
	pt.EdgeTess[2] = tess;

	pt.InsideTess = tess;
	//pt.InsideTess[1] = tess;

	return pt;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("CalcHSPatchConstants")]
DomainInputType main(InputPatch<HullInputType, NUM_CONTROL_POINTS> patch, uint i : SV_OutputControlPointID, uint PatchID : SV_PrimitiveID)
{
	DomainInputType Output;

	// Insert code to compute Output here
	Output.position = patch[i].position;
	Output.color = patch[i].color;

	return Output;
}
