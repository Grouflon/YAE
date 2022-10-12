uniform mat4 viewProj;
uniform mat4 model;

in vec3 inPosition;
in vec3 inColor;
in vec2 inTexCoord;

out vec3 fragColor;
out vec2 fragTexCoord;

void main()
{
    gl_Position = viewProj * model * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}
