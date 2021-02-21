//=============================================================================
// dirLightTex.fx by Frank Luna (C) 2004 All Rights Reserved.
//
// Uses a directional light plus texturing.
//=============================================================================

uniform extern float4x4 gWorld;
uniform extern float4x4 gWorldInvTrans;
uniform extern float4x4 gWVP;

uniform extern float3 gEyePosW;

struct OutputColorVS
{
	float4 posH : POSITION0;
	float3 posW : TEXCOORD0;
	float4 col  : COLOR0;
};
OutputColorVS PosColorVS(float3 posL : POSITION0, float4 col : COLOR0)
{
    // Zero out our output.
	OutputColorVS outVS = (OutputColorVS)0;
	
	// Transform vertex position to world space.
	outVS.posW  = mul(float4(posL, 1.0f), gWorld).xyz;
	
	// Transform to homogeneous clip space.
	outVS.posH = mul(float4(posL, 1.0f), gWVP);
	
	// Pass on color to be interpolated in rasterization.
	outVS.col = col;
	
	// Done--return the output.
    return outVS;
}

float4 PosColorPS(float4 col : COLOR0) : COLOR
{
	return col;
}

technique PosColTech
{
	pass P0
	{

        	// Specify the vertex and pixel shader associated with this pass.
        	vertexShader = compile vs_2_0 PosColorVS();
        	pixelShader  = compile ps_2_0 PosColorPS();
	}
}
