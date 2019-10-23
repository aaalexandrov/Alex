attribute vec4 aPosition;
attribute vec2 aTextureCoord;

uniform mat4 umTransform;

varying vec2 vTextureCoord;

void main() {
    gl_Position = umTransform * aPosition;
    vTextureCoord = aTextureCoord;
}
