#include "graphics.slang"
#include "lighting.slang"

struct MaterialUniform
{
    float4 color;
    float4 emissiveTint;

    float roughness;
    float metallic;
    float ao;
};

[[vk::binding(3, 0)]] ConstantBuffer<MaterialUniform> material;
[[vk::binding(4, 0)]] ConstantBuffer<Sampler2D> diffuseMap;
[[vk::binding(5, 0)]] ConstantBuffer<Sampler2D> normalMap;
[[vk::binding(6, 0)]] ConstantBuffer<Sampler2D> ormMap;

struct VSOutput
{
    float4 location : SV_Position;

    float3 tangentFragPos;
    float3 tangentViewDir;
    float3 tangentLightDirs[MAX_LIGHT_COUNT];
    float3 tangentHalfDirs[MAX_LIGHT_COUNT];
    
    float2 uv0;
    float4 color;
};

[shader("vertex")]
VSOutput main(VSInput input, uniform TransformUniform* pushData)
{
    VSOutput output;

    output.location = TransformVertex(pushData->model, input.location);
    output.uv0 = input.uv0;

    float3x3 tbn = CalculateTBN(input.normal, input.tangent, input.biTangent, pushData->normal);
    output.tangentFragPos = mul(tbn, output.location.xyz);
    output.tangentViewDir = normalize(mul(tbn, projectionView.cameraLocation) - output.tangentFragPos);
    
    for(int i = 0; i < MAX_LIGHT_COUNT; ++i)
    {
        output.tangentLightDirs[i] = TangentLightDirection(lights[i], output.tangentFragPos);
        output.tangentHalfDirs[i] = normalize(output.tangentLightDirs[i] + output.tangentViewDir);
    }

    output.color = input.color;

    return output;
}

[shader("fragment")]
float4 main(VSOutput input, uniform TransformUniform *pushData) : SV_Target
{
    float3 normal = MapNormals(normalMap, input.uv0).xyz;

    float3 color = diffuseMap.Sample(input.uv0).rgb;

    float3 ambient = 0.1 * color;

    float diff = max(dot(input.tangentLightDirs[0], normal), 0.0);
    float3 diffuse = diff * color;

    float roughness = 1.0 - material.roughness;
    float shininess = clamp(1.0 / (roughness * roughness), 1.0, 8192.0);
    float spec = pow(max(dot(normal, input.tangentHalfDirs[0]), 0.0), shininess);
    float3 specular = float3(0.2) * spec;

    return float4(ambient + specular + diffuse, 1.0);
}