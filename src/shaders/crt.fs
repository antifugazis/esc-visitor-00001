#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec2 renderSize;
uniform float scanlineIntensity;
uniform float vignetteStrength;
uniform float curvature;

out vec4 finalColor;

void main()
{
    vec2 uv = fragTexCoord;

    // Mild CRT barrel distortion to keep PS2-era instability subtle.
    vec2 centered = uv * 2.0 - 1.0;
    float r2 = dot(centered, centered);
    vec2 warped = centered * (1.0 + curvature * r2);
    vec2 sampleUV = warped * 0.5 + 0.5;

    if (sampleUV.x < 0.0 || sampleUV.x > 1.0 || sampleUV.y < 0.0 || sampleUV.y > 1.0)
    {
        finalColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    vec4 texel = texture(texture0, sampleUV) * fragColor * colDiffuse;

    // Horizontal scanline modulation based on low-res output rows.
    float row = sampleUV.y * renderSize.y;
    float line = 0.5 + 0.5 * sin(row * 3.14159265);
    float scanMul = mix(1.0, 0.86 + 0.14 * line, scanlineIntensity);
    texel.rgb *= scanMul;

    // Edge darkening.
    float vignette = 1.0 - dot(centered * 0.7, centered * 0.7);
    vignette = clamp(vignette, 0.0, 1.0);
    vignette = mix(1.0, vignette, vignetteStrength);
    texel.rgb *= vignette;

    finalColor = texel;
}
