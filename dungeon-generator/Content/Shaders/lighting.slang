struct LightUniform
{
	float4 location;
	float4 direction;
	float4 color;
	int32_t type;
};

struct SceneLightingUniform
{
    float4 ambientColor;
    float ambientStrength;
};

const static int MAX_LIGHT_COUNT = 8;
[[vk::binding(0, 0)]] ConstantBuffer<SceneLightingUniform> sceneLighting;
[[vk::binding(1, 0)]] ConstantBuffer<LightUniform> lights[MAX_LIGHT_COUNT];

float3 LightDirection(LightUniform light, float3 location)
{
    if (light.type == 0)
    {
        return light.direction.xyz;
	}

    return normalize(light.location.xyz - location);
}

float3 MapNormals(ConstantBuffer<Sampler2D> normalMap, float2 uv)
{
    return normalMap.Sample(uv).rgb;
}

float3x3 CalculateTBN(float4 normal, float4 tangent, float4 biTangent, float4x4 model)
{
    float3 t = normalize(mul(model, tangent)).rgb;
    float3 b = normalize(mul(model, biTangent)).rgb;
    float3 n = normalize(mul(model, normal)).rgb;

    return float3x3(t, b, n);
}