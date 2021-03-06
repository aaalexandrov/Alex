attribute vec4 aPosition;
attribute vec3 aNormal;
attribute vec2 aTextureCoord;

uniform mat4 umModelViewProj;
uniform mat4 umModel;

uniform vec3 uLightDir;
uniform vec3 uLightDiffuse;
uniform vec3 uLightAmbient;

uniform vec3 uMaterialDiffuse;
uniform vec4 uMaterialAmbient;  

varying vec2 vTextureCoord;
varying lowp vec4 vColor;

void main() {
    gl_Position = umModelViewProj * aPosition;
    vTextureCoord = aTextureCoord;
    
    vec3 norm = (umModel * vec4(aNormal, 0.0)).xyz;
    norm = normalize(norm);
    float NdotL = dot(norm, uLightDir);
    NdotL = max(NdotL, 0.0);
    vColor = vec4(uLightDiffuse * uMaterialDiffuse * NdotL + uLightAmbient * uMaterialAmbient.xyz, uMaterialAmbient.w);
}
