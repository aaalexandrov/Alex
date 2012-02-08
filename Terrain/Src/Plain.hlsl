
// Variables ----------------------------------------------------------

cbuffer cbPerMaterial {
	float4x4 g_mView, g_mProj;
}

cbuffer cbPerObject {
	float4x4 g_mWorld;
	float3x2 g_mTexTransform;

	float4 g_cMaterialDiffuse;
}

// Textures ---------------------------------------------------------

Texture2D<float4> g_txDiffuse;

// Samplers ---------------------------------------------------------

SamplerState g_sDiffuse;

// Shader Inputs ----------------------------------------------------

struct VS_INPUT {
	float3 vPosition: POSITION;
	float2 vTexCoord: TEXCOORD0;
};

struct PS_INPUT {
	float4 vPosition: SV_Position;
	float2 vTexCoord: TEXCOORD0;
};

struct PS_OUTPUT {
	float4 cColor: SV_Target;
};

// Shader Code ------------------------------------------------------

PS_INPUT PlainVS(VS_INPUT In)
{
	PS_INPUT Out;
	
	float4x4 mWorldViewProj = mul(g_mWorld, mul(g_mView, g_mProj));
	
	Out.vPosition = mul(float4(In.vPosition, 1), mWorldViewProj);
	Out.vTexCoord = mul(float3(In.vTexCoord, 1), g_mTexTransform);
	
	return Out;
}

PS_OUTPUT PlainPS(PS_INPUT In)
{
	PS_OUTPUT Out;
	float4 cTex = g_txDiffuse.Sample(g_sDiffuse, In.vTexCoord); 
	
    Out.cColor = g_cMaterialDiffuse * cTex;
    Out.cColor = g_cMaterialDiffuse;
    Out.cColor.a *= cTex.r;
    
	return Out;
}

