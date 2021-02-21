layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;
layout (location = 2) in vec2 texCoord;

out vec4 vertColor;
out vec2 vertTexCoord;

uniform PerInstance
{
	mat4 projection;
	mat4 view;
	mat4 model;

	vec4 emissiveColor;
};

void main()
{
	vertColor = color * emissiveColor;
	vertTexCoord = texCoord;
	gl_Position = projection * view * model * vec4(position, 1);
}
