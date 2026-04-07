#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

out vec4 finalColor;

float Bayer4x4(vec2 p)
{
    int x = int(mod(p.x, 4.0));
    int y = int(mod(p.y, 4.0));
    int i = x + y * 4;

    float bayer[16] = float[](
         0.0,  8.0,  2.0, 10.0,
        12.0,  4.0, 14.0,  6.0,
         3.0, 11.0,  1.0,  9.0,
        15.0,  7.0, 13.0,  5.0
    );

    return bayer[i] / 16.0;
}

void main()
{
    vec4 texel = texture(texture0, fragTexCoord) * fragColor * colDiffuse;

    // Heavy quantization + ordered dither for PS1/PS2 horror look.
    float d = (Bayer4x4(gl_FragCoord.xy) - 0.5) / 36.0;
    vec3 q = clamp(texel.rgb + d, 0.0, 1.0);
    vec3 c = floor(q * 24.0) / 24.0;
    finalColor = vec4(c, texel.a);
}
