#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform vec3 cameraPos;
uniform float fogNear;
uniform float fogFar;
uniform vec4 fogColor;

out vec4 finalColor;

void main()
{
    vec4 src = texture(texture0, fragTexCoord) * fragColor * colDiffuse;

    // Approximate distance fog from fragment depth for a constrained PS2 look.
    float z = gl_FragCoord.z / gl_FragCoord.w;
    float fogT = clamp((z - fogNear) / max(0.001, fogFar - fogNear), 0.0, 1.0);
    vec3 c = mix(src.rgb, fogColor.rgb, fogT);

    finalColor = vec4(c, src.a);
}
