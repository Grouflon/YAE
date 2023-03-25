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
in vec2 inTexCoord;
in vec3 inNormal;
in vec3 inColor;

out vec2 fragTexCoord;
out vec3 fragNormal;
out vec3 fragColor;

void main()
{
	gl_Position = viewProj * model * vec4(inPosition, 1.0);
	fragTexCoord = inTexCoord;
	fragNormal = (model * vec4(inNormal, 0.0)).xyz;
	fragColor = inColor;
}
