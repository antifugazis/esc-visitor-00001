#version 330

in vec2 fragTexCoord;
in vec3 fragNormal;
in vec3 fragPos;
in vec3 fragViewPos;

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
    vec3 l = normalize(-lightDir);
    vec3 v = normalize(fragViewPos - fragPos);
    vec3 h = normalize(l + v);
    
    // Diffuse lighting
    float ndl = max(dot(n, l), 0.0);
    
    // Specular (Blinn-Phong)
    float ndh = max(dot(n, h), 0.0);
    float specular = pow(ndh, 32.0) * 0.3;
    
    // Rim lighting for silhouettes
    float rim = 1.0 - max(dot(n, v), 0.0);
    rim = pow(rim, 3.0) * 0.2;
    
    // Combine lighting
    vec3 diffuse = albedo.rgb * (ambientColor * 0.6 + ndl * lightColor);
    vec3 spec = vec3(specular) * lightColor;
    vec3 rimLight = rim * lightColor * 0.3;
    
    vec3 lit = diffuse + spec + rimLight;

    if (useLightmap == 1)
    {
        vec3 lm = texture(lightmapTex, fragTexCoord).rgb;
        lit *= mix(vec3(1.0), lm * 1.6, 0.65);
    }

    // Tone mapping for more realistic lighting
    lit = lit / (lit + vec3(1.0));
    lit = pow(lit, vec3(1.0 / 2.2));

    finalColor = vec4(lit, albedo.a);
}
