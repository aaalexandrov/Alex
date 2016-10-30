#version 330

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;

out vec2 fragTexCoord;

uniform PerInstance
{
	mat4 projection;
	mat4 view;
	mat4 model;

	vec4 emissiveColor;
};

void main()
{
	fragTexCoord = texCoord;
	gl_Position = projection * view * model * vec4(position, 1);
}
