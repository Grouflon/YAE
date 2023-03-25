precision highp float;

uniform mat4 viewProj;
uniform mat4 model;

in vec3 inPosition;
in vec2 inTexCoord;
in vec3 inNormal;
in vec3 inColor;

out vec2 geomTexCoord;
out vec3 geomNormal;
out vec3 geomColor;
out vec4 geomWorldPos;

void main()
{
	geomWorldPos = model * vec4(inPosition, 1.0);
  gl_Position = viewProj * geomWorldPos;
  geomTexCoord = inTexCoord;
  geomNormal = (model * vec4(inNormal, 0.0)).xyz;
  geomColor = inColor;
}
