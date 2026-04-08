#version 330

in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;

uniform mat4 mvp;
uniform mat4 matModel;
uniform vec3 viewPos;

out vec2 fragTexCoord;
out vec3 fragNormal;
out vec3 fragPos;
out vec3 fragViewPos;

void main()
{
    fragTexCoord = vertexTexCoord;
    fragPos = vec3(matModel * vec4(vertexPosition, 1.0));
    fragNormal = mat3(transpose(inverse(matModel))) * vertexNormal;
    fragViewPos = viewPos;
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
