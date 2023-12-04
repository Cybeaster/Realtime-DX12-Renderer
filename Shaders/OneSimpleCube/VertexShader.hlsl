struct VertexPositionColor
{
	float3 pos : POSITION;
	float3 color : COLOR;
};

struct ModelViewProjection
{
	matrix MVP;
};

ConstantBuffer<ModelViewProjection> ModelViewProjectionCB : register(b0);

struct VertexShaderOutput
{
	float4 color : COLOR;
	float4 pos : SV_POSITION;
};

VertexShaderOutput main(VertexPositionColor IN)
{
	VertexShaderOutput OUT;
	OUT.pos = mul(ModelViewProjectionCB.MVP, float4(IN.pos, 1.0f));
	OUT.COLOR = float4(IN.color, 1.0f);
	return OUT;
}
