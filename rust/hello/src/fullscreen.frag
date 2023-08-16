#version 450

layout(location = 0) in vec2 tc;
layout(location = 0) out vec4 color;

layout (set = 0, binding = 0) uniform sampler samp;
layout (set = 0, binding = 1) uniform texture2D tex;

void main() {
    color = texture(sampler2D(tex, samp), tc);
}
