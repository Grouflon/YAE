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

in vec2 fragTexCoord;
in vec3 fragNormal;
in vec3 fragColor;

out vec4 outColor;

void main()
{
	outColor = texture(texSampler, fragTexCoord);
	/*
	vec3 color = texture(texSampler, fragTexCoord).xyz;
	float illumination = dot(normalize(vec3(-1.0, 0.0, 0.0)), fragNormal);
	outColor = vec4(color * ((illumination * 0.5) + 0.5), 1.0);
	*/
	//outColor = vec4(fragNormal, 1.f);
	//outColor = vec4(fragColor, 1.f);
}
