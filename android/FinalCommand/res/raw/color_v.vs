attribute vec4 aPosition;

uniform mat4 umModelViewProj;

void main() {
    gl_Position = umModelViewProj * aPosition;
}
