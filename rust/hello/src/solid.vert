#version 450

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tc;
layout (location = 2) in vec4 color;
 
layout (location = 0) out vec2 tc_vert;
layout (location = 1) out vec4 color_vert;
 
layout (set = 0, binding = 0, std140) uniform UniformData {
    mat4 view_proj;
};

void main() {
    gl_Position = view_proj * vec4(pos, 1);

    tc_vert = tc;
    color_vert = color;
}
