precision highp float;

uniform sampler2D texSampler;

in vec2 fragTexCoord;

out vec4 outColor;

void main()
{
	vec4 c = texture(texSampler, fragTexCoord);
	float gray = (c.x + c.y + c.z) / 3.0;
	c = vec4(gray, gray, gray, 1.0);
	outColor = c;
}
