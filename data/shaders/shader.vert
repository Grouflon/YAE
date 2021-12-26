#version 450

layout(binding = 0) uniform UniformBufferObject
{
	mat4 view;
	mat4 proj;
} ubo;

layout(push_constant) uniform Push {
  mat4 model;
} push;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 outFragColor;
layout(location = 1) out vec2 outFragTexCoord;

void main()
{
	gl_Position = ubo.proj * ubo.view * push.model * vec4(inPosition, 1.0);
	outFragColor = inColor;
	outFragTexCoord = inTexCoord;
}
