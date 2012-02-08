
// Variables ----------------------------------------------------------

cbuffer cbPerFrame {
	float4x4 g_mView, g_mProj;
	float4x4 g_mView_I;

	float3 g_vLightDirection;
	float3 g_cLightSpecular;
	float3 g_cLightDiffuse;
	float3 g_cLightAmbient;

	float  g_fLODDistance;
}

cbuffer cbPerMaterial {
	float4 g_cMaterialSpecular;
	float4 g_cMaterialDiffuse;
	float3 g_cMaterialAmbient;
	float3 g_cMaterialAverageColor;
	float  g_fMaterialID;
}

cbuffer cbPerObject {
	float4x4 g_mWorld;
	float3x2 g_mDiffTransform, g_mNormTransform;
}

// Textures ---------------------------------------------------------

Texture2D<float4> g_txDiffuse, g_txFar;
Texture2D<float2> g_txNormals;

// Samplers ---------------------------------------------------------

SamplerState g_sDiffuse, g_sNormals;

// Shader Inputs ----------------------------------------------------

struct VS_INPUT {
	float3 vPosition: POSITION;
	float2 vTexCoord: TEXCOORD0;
	float  fMaterial: TEXCOORD1;  
};

struct PS_INPUT {
	float4 vPosition : SV_Position;
	float2 vDiffCoord: TEXCOORD0;
	float2 vNormCoord: TEXCOORD1;
	float3 vWorldPos : TEXCOORD2;
	float  fAlpha    : TEXCOORD3;
};

struct PS_OUTPUT {
	float4 cColor: SV_Target;
};

// Shader Code ------------------------------------------------------

PS_INPUT TerrainVS(VS_INPUT In)
{
	PS_INPUT Out;
	
	float4x4 mWorldViewProj = mul(g_mWorld, mul(g_mView, g_mProj));
	
	Out.vPosition = mul(float4(In.vPosition, 1), mWorldViewProj);
	Out.vDiffCoord = mul(float3(In.vTexCoord, 1), g_mDiffTransform);
	Out.vNormCoord = mul(float3(In.vTexCoord, 1), g_mNormTransform);
	float4 vWorldPos = mul(float4(In.vPosition, 1), g_mWorld);
	Out.vWorldPos = vWorldPos.xyz / vWorldPos.w;
	Out.fAlpha = In.fMaterial >= g_fMaterialID;
	
	return Out;
}

PS_OUTPUT TerrainPS(PS_INPUT In)
{
	PS_OUTPUT Out;
	float4 cDiff = g_txDiffuse.Sample(g_sDiffuse, In.vDiffCoord); 
	float3 vNorm;
	float3 vRelative;
	float fAlpha, fDist, fMinDist = g_fLODDistance * 5 / 6, fMaxDist = g_fLODDistance;

	vRelative = In.vWorldPos - g_mView_I[3].xyz;
	fDist = length(vRelative);

	fAlpha = (fDist - fMinDist) / (fMaxDist - fMinDist);
	fAlpha = max(0, min(1, fAlpha));

//	cDiff.xyz = lerp(cDiff.xyz, g_cMaterialAverageColor, fAlpha);

//	cDiff = g_txFar.Sample(g_sNormals, In.vNormCoord);
//	cDiff = float4(g_txNormals.Sample(g_sNormals, In.vNormCoord), 0, 1);

	vNorm.xy = g_txNormals.Sample(g_sNormals, In.vNormCoord);
	vNorm.z = sqrt(1 - vNorm.x * vNorm.x - vNorm.y * vNorm.y);
	
	Out.cColor.xyz = g_cMaterialAmbient * g_cLightAmbient;
	float fNdotL = saturate(dot(g_vLightDirection, vNorm));
	Out.cColor.xyz += g_cMaterialDiffuse.xyz * g_cLightDiffuse * fNdotL;
	Out.cColor.xyz *= cDiff.xyz;
	float fRdotV = saturate(dot(reflect(g_vLightDirection, vNorm), normalize(-g_mView_I[3].xyz + In.vWorldPos)));
	Out.cColor.xyz += g_cMaterialSpecular.xyz * g_cLightSpecular * pow(fRdotV, g_cMaterialSpecular.w);
	Out.cColor.a = g_cMaterialDiffuse.a * cDiff.a * In.fAlpha;

//	Out.cColor.xyz = vNorm.xyz;
//	Out.cColor.xy = (vNorm.xy + 1) / 2;
//    Out.cColor.xyz = float3(In.vNormCoord.xy, 0);
//    Out.cColor.xyz = float3(g_txNormals.Sample(g_sNormals, In.vNormCoord).xy, 0);

	return Out;
}

PS_OUTPUT TerrainLODPS(PS_INPUT In)
{
	PS_OUTPUT Out;
	float4 cDiff; 
	float3 vNorm;

	cDiff = g_txFar.Sample(g_sNormals, In.vNormCoord);
//	cDiff = float4(g_txNormals.Sample(g_sNormals, In.vNormCoord), 0, 1);

	vNorm.xy = g_txNormals.Sample(g_sNormals, In.vNormCoord);
	vNorm.z = sqrt(1 - vNorm.x * vNorm.x - vNorm.y * vNorm.y);
	
	Out.cColor.xyz = g_cMaterialAmbient * g_cLightAmbient;
	float fNdotL = saturate(dot(g_vLightDirection, vNorm));
	Out.cColor.xyz += g_cMaterialDiffuse.xyz * g_cLightDiffuse * fNdotL;
	Out.cColor.xyz *= cDiff.xyz;
	float fRdotV = saturate(dot(reflect(g_vLightDirection, vNorm), normalize(-g_mView_I[3].xyz + In.vWorldPos)));
	Out.cColor.xyz += g_cMaterialSpecular.xyz * g_cLightSpecular * pow(fRdotV, g_cMaterialSpecular.w);
	Out.cColor.a = g_cMaterialDiffuse.a * cDiff.a * In.fAlpha;

	return Out;
}

