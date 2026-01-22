#version 450 core

in vec3 WorldPos;
in vec3 Normal;

out vec4 FragColor;

uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D  brdfLUT;

uniform vec3 camPos;

// Material constants
uniform vec3  uColor;
uniform float uMetallic;
uniform float uRoughness;
uniform float uAO;

const float PI = 3.14159265359;

// --- GGX helpers ---
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a  = roughness * roughness;
    float a2 = a * a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

void main()
{
    vec3 N = normalize(fs_in.Normal);
    vec3 V = normalize(camPos - fs_in.WorldPos);
    vec3 R = reflect(-V, N);

    vec3 F0 = mix(vec3(0.04), uColor, uMetallic);

    // Diffuse IBL
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse = irradiance * uColor;

    // Specular IBL
    float NdotV = max(dot(N, V), 0.0);
    vec3 F = fresnelSchlick(NdotV, F0);

    vec3 kS = F;
    vec3 kD = (1.0 - kS) * (1.0 - uMetallic);

    float maxMip = 5.0;
    vec3 prefiltered = textureLod(prefilterMap, R, uRoughness * maxMip).rgb;
    vec2 brdf = texture(brdfLUT, vec2(NdotV, uRoughness)).rg;

    vec3 specular = prefiltered * (F * brdf.x + brdf.y);

    vec3 ambient = (kD * diffuse + specular) * uAO;

    vec3 color = ambient;

    // Tone mapping
    color = color / (color + vec3(1.0));

    // Gamma correction
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, 1.0);
}
