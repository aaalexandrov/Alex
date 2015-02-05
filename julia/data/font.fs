#version 330

in  vec4 vertColor;
in  vec2 vertTexCoord;

out vec4 fragColor;

uniform sampler2D diffuseTexture;

void main()
{
	vec4 texel = texture2D(diffuseTexture, vertTexCoord);
    fragColor = vertColor * vec4(1.0, 1.0, 1.0, texel.r);
}
