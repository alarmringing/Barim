// vertex shader

#version 120

uniform mat4 modelViewProjectionMatrix;
attribute vec4 position;

void main(){
    gl_Position = modelViewProjectionMatrix * position;
}
