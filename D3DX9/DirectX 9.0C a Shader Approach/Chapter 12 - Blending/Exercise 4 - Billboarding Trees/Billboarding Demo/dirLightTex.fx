//=============================================================================
// dirLightTex.fx.
//
// Uses a directional light plus texturing, and performs billboarding.
//=============================================================================

uniform extern float4x4 gWorld;
uniform extern float4x4 gWorldInvTrans;
uniform extern float4x4 gWVP;
uniform extern float4x4 gViewProj;

uniform extern float4 gAmbientMtrl;
uniform extern float4 gAmbientLight;
uniform extern float4 gDiffuseMtrl;
uniform extern float4 gDiffuseLight;
uniform extern float4 gSpecularMtrl;
uniform extern float4 gSpecularLight;
uniform extern float  gSpecularPower;
uniform extern float3 gLightVecW;
uniform extern float3 gEyePosW;
uniform extern texture gTex;
uniform extern float3 gBBOffset;

sampler TexS = sampler_state
{
	Texture = <gTex>;
	MinFilter = Anisotropic;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	MaxAnisotropy = 8;
	AddressU  = WRAP;
    AddressV  = WRAP;
};
 
struct OutputVS
{
    float4 posH    : POSITION0;
    float4 diffuse : COLOR0;
    float4 spec    : COLOR1;
    float2 tex0    : TEXCOORD0;
};

struct OutputBillboardVS
{
    float4 posH    : POSITION0;
    float2 tex0	:	TEXCOORD0;
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
	
	// Done--return the output.
    return outVS;
}

float4 DirLightTexPS(float4 c : COLOR0, float4 spec : COLOR1, float2 tex0 : TEXCOORD0) : COLOR
{
	float4 texColor = tex2D(TexS, tex0);
	float3 diffuse = c.rgb * texColor.rgb;
    return float4(diffuse + spec.rgb, texColor.a*c.a); 
}

OutputBillboardVS AABillBoardVS(float3 posL : POSITION0,
		 float3 normalW : NORMAL0, 
                 float2 tex0 : TEXCOORD0)
{
    // Zero out our output.
	OutputBillboardVS outVS = (OutputBillboardVS)0;
	
      // Vertex world position without rotation applied to make
      // face the camera.
      float3 tempPosW = posL + gBBOffset;

      float3 look = gEyePosW - tempPosW;
      look.y = 0.0f;  // axis-aligned, so keep look in xz-plane.
      look = normalize(look);
      float3 up    = float3(0.0f, 1.0f, 0.0f);
      float3 right = cross(up, look);

      // Rotate to make face the camera.
      float3x3 R;
      R[0] = right;
      R[1] = up;
      R[2] = look;

      // Offset to world position.
      float3 posW = mul(posL, R) + gBBOffset;

      // Transform to homogeneous clip space.
      outVS.posH = mul(float4(posW, 1.0f), gViewProj);

      // Pass on texture coordinates to be interpolated in
      // rasterization.
      outVS.tex0 = tex0;

      // Done--return the output.
      return outVS;

}

float4 AABillBoardPS(float2 tex0 : TEXCOORD0) : COLOR
{
    float4 texColor = tex2D(TexS, tex0);
    return float4(texColor); 
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

technique BillBoard
{
	pass P0
    	{
        	// Specify the vertex and pixel shader associated with this pass.
        	vertexShader = compile vs_2_0 AABillBoardVS();
        	pixelShader  = compile ps_2_0 AABillBoardPS();
		AlphaTestEnable = true;
    		AlphaFunc = GreaterEqual;
    		AlphaRef = 200;

		CullMode = None;
	}
}
