layout (points) in;
layout (line_strip, max_vertices = 2) out;

uniform mat4 viewProj;
uniform mat4 model;

in vec2 geomTexCoord[];
in vec3 geomNormal[];
in vec3 geomColor[];
in vec4 geomWorldPos[];

out vec2 fragTexCoord;
out vec3 fragNormal;
out vec3 fragColor;
out vec4 fragWorldPos;

void main()
{
	float normalSize = 0.07;
	vec4 normal = (viewProj * model * vec4(geomNormal[0], 0.0)) * normalSize;

	gl_Position = gl_in[0].gl_Position;
	fragTexCoord = geomTexCoord[0];
	fragNormal = geomNormal[0];
	fragColor = geomColor[0];
	fragWorldPos = geomWorldPos[0];
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + normal;
	fragTexCoord = geomTexCoord[0];
	fragNormal = geomNormal[0];
	fragColor = geomColor[0];
	fragWorldPos = geomWorldPos[0];
	EmitVertex();
}