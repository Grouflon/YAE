#version 100
precision mediump float;

uniform sampler2D texture;

varying vec3 fragColor;
varying vec2 fragTexCoord;

void main()
{
	float alpha = texture2D(texture, fragTexCoord).w;
	gl_FragColor = vec4(fragColor.r, fragColor.g, fragColor.b, alpha);
	//gl_FragColor = vec4(alpha, 1.0, 1.0, 1.0);
	//gl_FragColor = vec4(fragTexCoord.y, fragTexCoord.y, 1.0, 1.0);
	//gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
