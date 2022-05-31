#version 100

uniform mat4 viewProj;
uniform mat4 model;

attribute vec3 inPosition;
attribute vec3 inColor;
attribute vec2 inTexCoord;

varying vec3 fragColor;
varying vec2 fragTexCoord;

void main()
{
    gl_Position = viewProj * model * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}
