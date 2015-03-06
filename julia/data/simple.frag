#version 330

in  vec2 fragTexCoord;

out vec4 fragColor;

uniform PerInstance
{
	mat4 projection;
	mat4 view;
	mat4 model;
    
    vec4 emissiveColor;
};
uniform sampler2D diffuseTexture;

void main()
{
	vec4 texel = texture2D(diffuseTexture, fragTexCoord);
    fragColor = emissiveColor * texel;  //vec4(1.0, 0.0, 0.0, 1.0);
}
