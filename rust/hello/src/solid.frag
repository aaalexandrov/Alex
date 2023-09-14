#version 450

layout (constant_id = 0) const bool alpha_mode = false;

layout (location = 0) in vec2 tc_vert;
layout (location = 1) in vec4 color_vert;
 
layout (location = 0) out vec4 color;

layout (set = 0, binding = 0, std140) uniform UniformData {
    mat4 view_proj;
};

layout (set = 0, binding = 1) uniform sampler samp;
layout (set = 0, binding = 2) uniform texture2D tex;

void main() {
    vec4 tex = texture(sampler2D(tex, samp), tc_vert);
    if (alpha_mode)
        tex = vec4(1, 1, 1, tex.r);
    color = tex * color_vert;
}
