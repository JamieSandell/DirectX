//=============================================================================
// dirLightTex.fx by Frank Luna (C) 2004 All Rights Reserved.
//
// Uses a directional light plus texturing.
//=============================================================================

uniform extern float4x4 gWorld;
uniform extern float4x4 gWorldInvTrans;
uniform extern float4x4 gWVP;

uniform extern float4 gAmbientMtrl;
uniform extern float4 gAmbientLight;
uniform extern float4 gDiffuseMtrl;
uniform extern float4 gDiffuseLight;
uniform extern float4 gSpecularMtrl;
uniform extern float4 gSpecularLight;
uniform extern float  gSpecularPower;
uniform extern float3 gLightVecW;
uniform extern float3 gEyePosW;
uniform extern texture gTex1;
uniform extern texture gTex2;
uniform extern float gSpeed;
uniform extern float gSpeed2;
uniform extern float gTime;

sampler TexS1 = sampler_state
{
	Texture = <gTex1>;
	MinFilter = Anisotropic;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	MaxAnisotropy = 8;
	AddressU  = BORDER;
        AddressV  = BORDER;
	BorderColor = 0xff0000ff;
};

sampler TexS2 = sampler_state
{
	Texture = <gTex2>;
	MinFilter = Anisotropic;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	MaxAnisotropy = 8;
	AddressU  = BORDER;
        AddressV  = BORDER;
	BorderColor = 0xff000000;
};

float2x2 RotationMatrix(float rotation)  
{  
    float c = cos(rotation);  
    float s = sin(rotation);  
 
    return float2x2(c, -s, s ,c);  
}
 
struct OutputVS
{
    float4 posH    : POSITION0;
    float4 diffuse : COLOR0;
    float4 spec    : COLOR1;
    float2 tex0    : TEXCOORD0;
    float2 tex1	   : TEXCOORD1;
};

OutputVS DirLightTexVS(float3 posL : POSITION0, float3 normalL : NORMAL0, float2 tex0: TEXCOORD0)
{
    // Zero out our output.
	OutputVS outVS = (OutputVS)0;
	
	// Transform normal to world space.
	float3 normalW = mul(float4(normalL, 0.0f), gWorldInvTrans).xyz;
	normalW = normalize(normalW);
	
	// Transform vertex position to world space.
	float3 posW  = mul(float4(posL, 1.0f), gWorld).xyz;
	
	//=======================================================
	// Compute the color: Equation 10.3.
	
	// Compute the vector from the vertex to the eye position.
	float3 toEye = normalize(gEyePosW - posW);
	
	// Compute the reflection vector.
	float3 r = reflect(-gLightVecW, normalW);
	
	// Determine how much (if any) specular light makes it into the eye.
	float t  = pow(max(dot(r, toEye), 0.0f), gSpecularPower);
	
	// Determine the diffuse light intensity that strikes the vertex.
	float s = max(dot(gLightVecW, normalW), 0.0f);
	
	// Compute the ambient, diffuse and specular terms separatly. 
	float3 spec = t*(gSpecularMtrl*gSpecularLight).rgb;
	float3 diffuse = s*(gDiffuseMtrl*gDiffuseLight).rgb;
	float3 ambient = gAmbientMtrl*gAmbientLight;
	
	// Sum all the terms together and copy over the diffuse alpha.
	outVS.diffuse.rgb = ambient + diffuse;
	outVS.diffuse.a   = gDiffuseMtrl.a;
	outVS.spec = float4(spec, 0.0f);
	//=======================================================
	
	// Transform to homogeneous clip space.
	outVS.posH = mul(float4(posL, 1.0f), gWVP);
	
	// Pass on texture coordinates to be interpolated in rasterization.
	outVS.tex0 = tex0;
	outVS.tex0 -= float2(0.5f, 0.5f);
	outVS.tex0 = mul(outVS.tex0, RotationMatrix(gSpeed*gTime));
	outVS.tex0 += float2(0.5f, 0.5f);

	outVS.tex1 = tex0;
	outVS.tex1 -= float2(0.5f, 0.5f);
	outVS.tex1 = mul(outVS.tex1, RotationMatrix(gSpeed2*gTime));
	outVS.tex1 += float2(0.5f, 0.5f);
	
	// Done--return the output.
    return outVS;
}

float4 DirLightTexPS(float4 c : COLOR0, float4 spec : COLOR1, float2 tex0 : TEXCOORD0, float2 tex1 : TEXCOORD1) : COLOR
{
	float3 texColor1 = tex2D(TexS1, tex0).rgb;
	float3 texColor2 = tex2D(TexS2, tex1).rgb;
	float3 diffuse = c.rgb * (texColor1 * texColor2);
    	return float4(diffuse + spec.rgb, c.a);
}

technique DirLightTexTech
{
    pass P0
    {
        // Specify the vertex and pixel shader associated with this pass.
        vertexShader = compile vs_2_0 DirLightTexVS();
        pixelShader  = compile ps_2_0 DirLightTexPS();
    }
}
