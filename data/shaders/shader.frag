precision highp float;

/*
uniform sampler2D texture;

varying vec3 fragColor;
varying vec2 fragTexCoord;
*/

/*
#extension GL_ARB_separate_shader_objects : enable
*/

uniform sampler2D texSampler;

in vec3 fragColor;
in vec2 fragTexCoord;

out vec4 outColor;

void main()
{
	outColor = texture(texSampler, fragTexCoord);
	//gl_FragColor = texture2D(texture, fragTexCoord);
	//gl_FragColor.x = gl_FragColor.x + 0.5;
}
