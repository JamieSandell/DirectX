//=============================================================================
// TerrainMultiTex.fx by Frank Luna (C) 2004 All Rights Reserved.
//
// Blends three textures together with a blend map.
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
uniform extern texture gTex0;
uniform extern texture gTex1;
uniform extern texture gTex2;
uniform extern texture gBlendMap;

// Use Anisotropic filtering since when we are low to the ground, the 
// ground plane is near a 90 degree angle with our view direction.
sampler Tex0S = sampler_state
{
	Texture = <gTex0>;
	MinFilter = Anisotropic;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	MaxAnisotropy = 8;
	AddressU  = WRAP;
    AddressV  = WRAP;
};

sampler Tex1S = sampler_state
{
	Texture = <gTex1>;
	MinFilter = Anisotropic;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	MaxAnisotropy = 8;
	AddressU  = WRAP;
    AddressV  = WRAP;
};

sampler Tex2S = sampler_state
{
	Texture = <gTex2>;
	MinFilter = Anisotropic;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	MaxAnisotropy = 8;
	AddressU  = WRAP;
    AddressV  = WRAP;
};

sampler BlendMapS = sampler_state
{
	Texture = <gBlendMap>;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	AddressU  = WRAP;
    AddressV  = WRAP;
};
 
struct OutputColorVS
{
    float4 posH         : POSITION0;
    float4 diffuse      : COLOR0;
    float4 spec         : COLOR1;
};

struct OutputBaseTextureVS
{
    float2 tiledTexC    : TEXCOORD0;
	float4 posH	: POSITION0;
};

struct OutputSecondTextureVS
{
    float2 tiledTexC    : TEXCOORD0;
	float4 posH	: POSITION0;
};

struct OutputThirdTextureVS
{
    float2 tiledTexC    : TEXCOORD0;
	float4 posH	: POSITION0;
};

OutputColorVS ColorVS(float3 posL : POSITION0, 
                           float3 normalL : NORMAL0 )
{
    // Zero out our output.
	OutputColorVS outVS = (OutputColorVS)0;
	
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
	
	
	// Done--return the output.
    return outVS;
}

float4 ColorPS(float4 diffuse : COLOR0, float4 spec    : COLOR1) : COLOR
{
    
    // Sum the colors and modulate with the lighting color.
    
    return float4(spec.rgb, diffuse.a);
}

OutputBaseTextureVS BaseTextureVS(float3 posL : POSITION0, float2 tiledTexC : TEXCOORD0)
{
	// Zero out our output.
	OutputBaseTextureVS outVS = (OutputBaseTextureVS)0;
	outVS.tiledTexC    = tiledTexC * 16.0f; // Scale tex-coord to tile 16 times.
	outVS.posH = mul(float4(posL, 1.0f), gWVP);	
    return outVS;
}

float4 BaseTexturePS(float2 tiledTexC : TEXCOORD0) : COLOR
{
	// Layer maps are tiled
    	float4 c0 = tex2D(Tex0S, tiledTexC);
	tiledTexC = tiledTexC / 16.0f;
	float4 c1 = tex2D(BlendMapS, tiledTexC);
	float totalInverse = 1.0f / (c1.r + c1.g + c1.b);
	c0.a = c1.r * totalInverse;
    
    	return float4(c0);
}

OutputSecondTextureVS SecondTextureVS(float3 posL : POSITION0, float2 tiledTexC : TEXCOORD0)
{
	// Zero out our output.
	OutputSecondTextureVS outVS = (OutputSecondTextureVS)0;
	outVS.tiledTexC    = tiledTexC * 16.0f; // Scale tex-coord to tile 16 times.
	outVS.posH = mul(float4(posL, 1.0f), gWVP);	
    return outVS;
}

float4 SecondTexturePS(float2 tiledTexC : TEXCOORD0) : COLOR
{
	// Layer maps are tiled
    	float4 c0 = tex2D(Tex1S, tiledTexC);
	tiledTexC = tiledTexC / 16.0f;
	float4 c1 = tex2D(BlendMapS, tiledTexC);
	float totalInverse = 1.0f / (c1.r + c1.g + c1.b);
	c0.a = c1.g * totalInverse;
    
    	return float4(c0);
}

OutputThirdTextureVS ThirdTextureVS(float3 posL : POSITION0, float2 tiledTexC : TEXCOORD0)
{
	// Zero out our output.
	OutputThirdTextureVS outVS = (OutputThirdTextureVS)0;
	outVS.tiledTexC    = tiledTexC * 16.0f; // Scale tex-coord to tile 16 times.
	outVS.posH = mul(float4(posL, 1.0f), gWVP);	
    return outVS;
}

float4 ThirdTexturePS(float2 tiledTexC : TEXCOORD0) : COLOR
{
	// Layer maps are tiled
    	float4 c0 = tex2D(Tex2S, tiledTexC);
	tiledTexC = tiledTexC / 16.0f;
	float4 c1 = tex2D(BlendMapS, tiledTexC);
	float totalInverse = 1.0f / (c1.r + c1.g + c1.b);
	c0.a = c1.b * totalInverse;
    
    	return float4(c0);
}

technique TerrainMultiTexTech
{
    	pass PBaseTexture
    	{
        	// Specify the vertex and pixel shader associated with this pass.
        	vertexShader = compile vs_2_0 BaseTextureVS();
        	pixelShader  = compile ps_2_0 BaseTexturePS();
    	}

    	pass PSecondTexture
    	{
		AlphaBlendEnable = True;
		SrcBlend = SrcAlpha; 
		DestBlend = InvSrcAlpha; 
        	// Specify the vertex and pixel shader associated with this pass.
        	vertexShader = compile vs_2_0 SecondTextureVS();
        	pixelShader  = compile ps_2_0 SecondTexturePS();
    	}

    	pass PThirdTexture
    	{
		AlphaBlendEnable = True;
		SrcBlend = SrcAlpha; 
		DestBlend = InvSrcAlpha; 
        	// Specify the vertex and pixel shader associated with this pass.
        	vertexShader = compile vs_2_0 ThirdTextureVS();
        	pixelShader  = compile ps_2_0 ThirdTexturePS();
    	}

	pass PColor
	{
		AlphaBlendEnable = True;
		SrcBlend = One; 
		DestBlend = One;
		// Specify the vertex and pixel shader associated with this pass.
        	vertexShader = compile vs_2_0 ColorVS();
        	pixelShader  = compile ps_2_0 ColorPS();
	}	
}
