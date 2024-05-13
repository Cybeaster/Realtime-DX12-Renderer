#include "Common.hlsl"

struct GSInput {
	float4 PosW : SV_POSITION ;
	nointerpolation float3 Color : COLOR;
};

struct VertexIn{
	float3 PosL : POSITION;
};


GSInput VS(VertexIn VIn, uint InstanceID
			: SV_InstanceID)
{
	GSInput output = (GSInput)0;
	InstanceData inst = gInstanceData[InstanceID];
	float4x4 world = inst.World;
	float4 posW = mul(float4(VIn.PosL, 1.0f), world);
	output.PosW = posW;
	output.Color = inst.OverrideColor;
	return output;
}


[maxvertexcount(6)]
void GS(triangle GSInput input[3], inout LineStream<GSInput> OutputStream)
{

		OutputStream.Append( input[0]);
		OutputStream.Append( input[1]);
		OutputStream.Append( input[2]);

}


float4 PS(GSInput input) : SV_Target
{
	return float4(1,0,0,1);
}