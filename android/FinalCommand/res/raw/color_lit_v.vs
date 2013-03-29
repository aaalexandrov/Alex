attribute vec4 aPosition;
attribute vec3 aNormal;

uniform mat4 umModelViewProj;
uniform mat4 umModel;

uniform vec3 uLightDir;
uniform vec3 uLightDiffuse;
uniform vec3 uLightAmbient;

uniform vec3 uMaterialDiffuse;
uniform vec3 uMaterialAmbient;  

varying vec4 vColor;

void main() {
    gl_Position = umModelViewProj * aPosition;
    
    vec3 norm = (umModel * vec4(aNormal, 0.0)).xyz;
    norm = normalize(norm);
    float NdotL = dot(norm, uLightDir);
    NdotL = max(NdotL, 0.0);
    vColor = vec4(uLightDiffuse * uMaterialDiffuse * NdotL + uLightAmbient * uMaterialAmbient, 1.0);
}
