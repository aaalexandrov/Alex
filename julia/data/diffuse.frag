#version 330

in  vec3 fragNorm;
in  vec2 fragTexCoord;

out vec4 fragColor;

uniform PerInstance 
{
	mat4 projection;
	mat4 view;
	mat4 model;
    mat3 modelIT;
    
    vec3 sunDirection;
    vec3 sunColor;
    vec4 ambientColor;

    vec4 ambientMaterial;
    vec3 diffuseMaterial;
};

uniform sampler2D diffuseTexture;

void main()
{
	vec4 texel = texture2D(diffuseTexture, fragTexCoord);
    vec3 N = normalize(fragNorm);
    float NdotL = max(0, dot(N, sunDirection));
    vec3 diffuse = NdotL * sunColor * diffuseMaterial;
    fragColor = (ambientColor * ambientMaterial + vec4(diffuse, 0)) * texel;
}
