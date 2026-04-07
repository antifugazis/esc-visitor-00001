#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform float gradeStrength;

out vec4 finalColor;

void main()
{
    vec4 src = texture(texture0, fragTexCoord) * fragColor * colDiffuse;

    float luma = dot(src.rgb, vec3(0.299, 0.587, 0.114));
    vec3 murky = vec3(
        luma * 0.92 + src.r * 0.18,
        luma * 0.86 + src.g * 0.24,
        luma * 0.50 + src.b * 0.08
    );

    // Warmer but less yellow-heavy tint.
    murky *= vec3(0.96, 0.94, 0.86);

    // Lift blacks a bit and crush highlights slightly.
    murky = mix(vec3(0.06), murky, 0.92);
    murky = pow(murky, vec3(1.08));

    vec3 graded = mix(src.rgb, murky, clamp(gradeStrength, 0.0, 1.0));

    finalColor = vec4(graded, src.a);
}
