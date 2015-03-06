#version 330

layout (location = 0) in vec4 vert;
layout (location = 1) in vec3 norm;
layout (location = 2) in vec2 texCoord;

out vec3 fragNorm;
out vec2 fragTexCoord;

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
    //fragNorm = vec3(model * vec4(norm, 0));
    fragNorm = modelIT * norm;
	fragTexCoord = texCoord;
    gl_Position = projection * view * model * vert;
}
