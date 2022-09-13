#version 100
precision mediump float;

uniform sampler2D texture;

varying vec3 fragColor;
varying vec2 fragTexCoord;

void main()
{
	vec4 pixel = texture2D(texture, fragTexCoord);
	// OpenGL ES puts the alpha in the a channel, but OpenGL 3.3 puts it in the red channel
#ifdef OPENGL_ES
	gl_FragColor = vec4(fragColor.r, fragColor.g, fragColor.b, pixel.w);
#else
	gl_FragColor = vec4(fragColor.r, fragColor.g, fragColor.b, pixel.x);
#endif
}
