precision highp float;

uniform mat4 viewProj;
uniform mat4 model;

/*
attribute vec3 inPosition;
attribute vec3 inColor;
attribute vec2 inTexCoord;

varying vec3 fragColor;
varying vec2 fragTexCoord;
*/

/*layout(binding = 0) uniform UniformBufferObject
{
	mat4 view;
	mat4 proj;
} ubo;

layout(push_constant) uniform Push {
  mat4 model;
} push;
*/

in vec3 inPosition;
in vec3 inColor;
in vec2 inTexCoord;

out vec3 fragColor;
out vec2 fragTexCoord;

void main()
{
	gl_Position = viewProj * model * vec4(inPosition, 1.0);
	fragColor = inColor;
	fragTexCoord = inTexCoord;
}
