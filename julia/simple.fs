#version 330

in  vec2 fragTexCoord;

out vec4 fragColor;

uniform vec4      emissiveColor;
uniform sampler2D diffuseTexture;

void main()
{
	vec4 texel = texture2D(diffuseTexture, fragTexCoord);
    fragColor = emissiveColor * texel;  //vec4(1.0, 0.0, 0.0, 1.0);
}