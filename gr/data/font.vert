#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 transform;
	vec4 fontColor;
} ubo;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec4 inColor;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec4 fragColor;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position = ubo.transform * vec4(inPosition, 0, 1.0);
	//gl_Position = vec4(inPosition / 150 - 1, 0.0, 1);
	fragTexCoord = inTexCoord;
    fragColor = inColor * ubo.fontColor;
}