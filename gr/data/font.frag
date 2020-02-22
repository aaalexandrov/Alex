#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec4 fragColor;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

void main() {
	float texColor = texture(texSampler, fragTexCoord).r;
    outColor = fragColor * texColor;
	//outColor = vec4(1,1,1,1);
}