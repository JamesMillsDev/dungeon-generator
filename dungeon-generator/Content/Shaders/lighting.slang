#include "global.slang"

struct LightUniform
{
	float4 location;
	float4 direction;
	float4 color;

    float intensity;
    float constant;
    float linear;
    float quadratic;
    float cutOff;
    float outerCutOff;
    
    int32_t type;
    int32_t enabled;
};

struct SceneLightingUniform
{
    float4 ambientColor;
    float ambientStrength;
};

const static int MAX_LIGHT_COUNT = 8;
[[vk::binding(1, 0)]] ConstantBuffer<SceneLightingUniform> sceneLighting;
[[vk::binding(2, 0)]] ConstantBuffer<LightUniform> lights[MAX_LIGHT_COUNT];

float3 LightDirection(LightUniform light, float3 location)
{
    // The light is disabled, so return no direction
    if (light.enabled == 0)
    {
        return float3(0, 1, 0);
    }

    // Directional light
    if (light.type == 0)
    {
        return normalize(light.direction.xyz);
	}

    return normalize(light.location.xyz - location);
}

float3 TangentLightDirection(LightUniform light, float3 tangentViewLoc)
{
    // The light is disabled, so return no direction
    if (light.enabled == 0)
    {
        return float3(0, 1, 0);
    }

    // Directional light
    if (light.type == 0)
    {
        return normalize(light.direction.xyz);
	}

    return normalize(light.location.xyz - tangentViewLoc);
}

float4 MapNormals(ConstantBuffer<Sampler2D> normalMap, float2 uv)
{
    return normalize(normalMap.SampleLevel(uv, 1.0) * 2.0 - 1.0);
}

float3x3 CalculateTBN(float4 normal, float4 tangent, float4 biTangent, float4x4 normalMatrix)
{
    float3 t = normalize(mul(normalMatrix, tangent)).xyz;
    float3 b = normalize(mul(normalMatrix, biTangent)).xyz;
    float3 n = normalize(mul(normalMatrix, normal)).xyz;

    return float3x3(t, b, n);
}

float4 FresnelSchlick(float cosTheta, float4 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(float NdotL, float NdotV, float roughness)
{
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);

    return ggx1 * ggx2;
}

float4 CalculateRadiance(float3 worldLoc, float3x3 tbn, float4 orm, float4 albedo)
{
    float4 Lo = float4(0.0);
    float3 V = mul(tbn, projectionView.cameraLocation);
    float3 N = tbn[2];

    float roughness = orm.g;
    float metallic = orm.b;

    // Pre-calculate common terms for efficiency
    float4 F0 = float4(0.04);
    F0 = lerp(F0, albedo, metallic);
    float NdotV = max(dot(N, V), 0.0);

    for(int i = 0; i < MAX_LIGHT_COUNT; ++i)
    {
        LightUniform light = lights[i];

        if(light.enabled == 0)
        {
            continue;
        }

        // calculate per-light radiance
        float3 L = mul(tbn, LightDirection(light, worldLoc));
        float3 H = normalize(V + L);

        float NdotL = max(dot(N, L), 0.0);

        float distance = length(L);
        float atten = 1.0 / (distance * distance);
        float4 radiance = (light.color * light.intensity) * atten;

        // cook-torrance brdf
        float cosTheta = max(dot(H, V), 0.0);
        float4 F = FresnelSchlick(cosTheta, F0);

        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(NdotL, NdotV, roughness);

        float4 numerator = NDF * G * F;
        float denominator = 4.0 * NdotV * NdotV + 0.0001;
        float4 specular = numerator / denominator;

        // add to outgoing radiance Lo
        float4 kS = F;
        float4 kD = float4(1.0) - kS;
        kD *= 1.0 - metallic;

        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    return Lo;
}