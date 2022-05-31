#version 100
precision mediump float;

uniform sampler2D texture;

varying vec3 fragColor;
varying vec2 fragTexCoord;

/*#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;*/

void main()
{
	gl_FragColor = texture2D(texture, fragTexCoord);
	//gl_FragColor.x = gl_FragColor.x + 0.5;
}
