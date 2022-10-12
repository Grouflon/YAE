precision highp float;

uniform sampler2D texSampler;

in vec3 fragColor;
in vec2 fragTexCoord;

out vec4 outColor;

void main()
{
	vec4 pixel = texture(texSampler, fragTexCoord);
	// OpenGL ES puts the alpha in the a channel, but OpenGL 3.3 puts it in the red channel
#ifdef OPENGL_ES
	outColor = vec4(fragColor.r, fragColor.g, fragColor.b, pixel.w);
#else
	outColor = vec4(fragColor.r, fragColor.g, fragColor.b, pixel.x);
#endif
}
