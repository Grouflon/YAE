layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec2 geomTexCoord[];
in vec3 geomNormal[];
in vec3 geomColor[];
in vec4 geomWorldPos[];

out vec2 fragTexCoord;
out vec3 fragNormal;
out vec3 fragColor;
out vec4 fragWorldPos;

out vec3 barycentricCoordinates;

void main()
{
	vec3 p0 = geomWorldPos[0].xyz;
	vec3 p1 = geomWorldPos[1].xyz;
	vec3 p2 = geomWorldPos[2].xyz;
	vec3 triangleNormal = normalize(cross(p1 - p0, p2 - p0));

	gl_Position = gl_in[0].gl_Position;
	fragTexCoord = geomTexCoord[0];
	fragNormal = triangleNormal;
	fragColor = geomColor[0];
	fragWorldPos = geomWorldPos[0];
	barycentricCoordinates = vec3(1.0, 0.0, 0.0);
	EmitVertex();

	gl_Position = gl_in[1].gl_Position;
	fragTexCoord = geomTexCoord[1];
	fragNormal = triangleNormal;
	fragColor = geomColor[1];
	fragWorldPos = geomWorldPos[1];
	barycentricCoordinates = vec3(0.0, 1.0, 0.0);
	EmitVertex();

	gl_Position = gl_in[2].gl_Position;
	fragTexCoord = geomTexCoord[2];
	fragNormal = triangleNormal;
	fragColor = geomColor[2];
	fragWorldPos = geomWorldPos[2];
	barycentricCoordinates = vec3(0.0, 0.0, 1.0);
	EmitVertex();
}