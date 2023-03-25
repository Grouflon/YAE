precision highp float;

// Resources here:
// https://catlikecoding.com/unity/tutorials/advanced-rendering/flat-and-wireframe-shading/

uniform sampler2D texSampler;

in vec2 fragTexCoord;
in vec3 fragNormal;
in vec3 fragColor;
in vec4 fragWorldPos;

out vec4 outColor;

void main()
{
	outColor = vec4(1.0, 0.0, 0.0, 1.0);
}
