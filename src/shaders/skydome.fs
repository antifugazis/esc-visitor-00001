#version 330

in vec2 fragTexCoord;

uniform sampler2D texture0;

out vec4 finalColor;

void main()
{
    vec3 sky = texture(texture0, fragTexCoord).rgb;
    finalColor = vec4(sky, 1.0);
}
