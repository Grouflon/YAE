precision highp float;

// Resources here:
// https://catlikecoding.com/unity/tutorials/advanced-rendering/flat-and-wireframe-shading/

uniform sampler2D texSampler;

in vec2 fragTexCoord;
in vec3 fragNormal;
in vec3 fragColor;
in vec4 fragWorldPos;

in vec3 barycentricCoordinates;

out vec4 outColor;

void main()
{
	vec3 ambient = vec3(0.5, 0.5, 0.55);
	vec3 diffuse = vec3(1.0, 0.99, 0.95);
	vec3 lightDirection = normalize(vec3(0.0, -0.5, -1.0));

	float wireframeThickness = 0.3;
	float wireframeSmooth = 0.1;
	vec3 wireframeColor = vec3(0.1, 0.1, 0.1);

	/*
	// Get flat normal without the geometry shader
	vec3 dx = dFdx(fragWorldPos.xyz);
	vec3 dy = dFdy(fragWorldPos.xyz);
	vec3 flatNormal = normalize(cross(dy, dx));
	*/

	vec3 barys = barycentricCoordinates;
	vec3 deltas = abs(dFdx(barys)) + abs(dFdy(barys));
	vec3 smoothing = deltas * wireframeSmooth;
	vec3 thickness = deltas * wireframeThickness;
	barys = smoothstep(thickness, thickness + smoothing, barys);
	float minBary = min(barys.x, min(barys.y, barys.z));

	float light = dot(-fragNormal, lightDirection);
	vec3 c = (1.0 - light) * ambient + light * diffuse;
	outColor = vec4(mix(vec3(wireframeColor), c, minBary), 1.0);
}
