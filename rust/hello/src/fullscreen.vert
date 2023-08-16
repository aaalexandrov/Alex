#version 450

layout(location = 0) out vec2 tc;

void main() {
    tc = vec2(
        gl_VertexIndex % 2,
        gl_VertexIndex / 2
    );
    gl_Position = vec4(
        mix(-1, 1, tc.x),
        mix(-1, 1, tc.y),
        0,
        1
    );
}
