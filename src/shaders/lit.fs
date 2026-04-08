#version 330

in vec2 fragTexCoord;
in vec3 fragNormal;
in vec3 fragPos;

uniform sampler2D texture0;
uniform sampler2D lightmapTex;
uniform int useLightmap;
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 ambientColor;

out vec4 finalColor;

void main()
{
    vec4 albedo = texture(texture0, fragTexCoord);
    if (albedo.a < 0.02) discard;

    vec3 n = normalize(fragNormal);
    float ndl = max(dot(n, normalize(-lightDir)), 0.0);
    vec3 lit = albedo.rgb * (ambientColor + ndl * lightColor);

    if (useLightmap == 1)
    {
        vec3 lm = texture(lightmapTex, fragTexCoord).rgb;
        lit *= mix(vec3(1.0), lm * 1.6, 0.65);
    }

    finalColor = vec4(lit, albedo.a);
}
