precision highp float;

uniform mat4 viewProj;
uniform mat4 model;

in vec3 inPosition;
in vec2 inTexCoord;
in vec3 inNormal;
in vec3 inColor;

out vec2 fragTexCoord;
out vec3 fragNormal;
out vec3 fragColor;

void main()
{
    gl_Position = viewProj * model * vec4(inPosition, 1.0);
    fragTexCoord = inTexCoord;
    fragNormal = (model * vec4(inNormal, 0.0)).xyz;
    fragColor = inColor;
}
