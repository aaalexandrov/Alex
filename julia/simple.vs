#version 330

layout (location = 0) in vec4 vert;
layout (location = 1) in vec2 texCoord;

out vec2 fragTexCoord;

uniform PerInstance
{
	mat4 projection;
	mat4 view;
	mat4 model;
};

void main()
{
	fragTexCoord = texCoord;
	
    gl_Position = projection * view * model * vert;
}