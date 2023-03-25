precision highp float;

uniform sampler2D texSampler;

in vec2 fragTexCoord;
in vec3 fragNormal;
in vec3 fragColor;

out vec4 outColor;

void main()
{
	vec4 pixel = texture(texSampler, fragTexCoord);
	// OpenGL ES puts the alpha in the a channel, but OpenGL 3.3 puts it in the red channel
#ifdef OPENGL_ES
	outColor = vec4(fragColor, pixel.w);
#else
	outColor = vec4(fragColor, pixel.x);
	//outColor = vec4(fragNormal, pixel.x);
#endif
}
