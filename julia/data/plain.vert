#version 330

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out vec3 fragNorm;

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

void main()
{
	fragNorm = modelIT * normal;
	gl_Position = projection * view * model * vec4(position, 1);
}
