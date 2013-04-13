attribute vec4 aPosition;
attribute vec4 aColor;

uniform mat4 umModelViewProj;

varying lowp vec4 vColor;

void main() {
    gl_Position = umModelViewProj * aPosition;
    
    vColor = aColor;
}
