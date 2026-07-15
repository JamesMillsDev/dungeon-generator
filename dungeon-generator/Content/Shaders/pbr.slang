#include "global.slang"
#include "graphics.slang"
#include "lighting.slang"

struct MaterialUniform
{
    float4 color;
    float4 emissiveTint;
    float4 specularColor;

    float roughness;
    float metallic;
    float specularStrength;
};

[[vk::binding(2, 0)]] ConstantBuffer<MaterialUniform> material;
[[vk::binding(3, 0)]] ConstantBuffer<Sampler2D> diffuseMap;
[[vk::binding(4, 0)]] ConstantBuffer<Sampler2D> normalMap;
[[vk::binding(5, 0)]] ConstantBuffer<Sampler2D> ormMap;

struct VSOutput
{
    float4 location : SV_Position;
    float3x3 tbn;
    float3 normal;

    float2 uv0;
    float4 color;
    float3 objectSpaceLocation;
};

[shader("vertex")]
VSOutput main(VSInput input, uniform ProjectionViewModelUniform *pushData)
{
    VSOutput output;

    output.location = TransformVertex(pushData, input.location);
    output.uv0 = input.uv0;
    output.tbn = CalculateTBN(input.normal, input.tangent, input.biTangent, pushData->model);

    output.color = input.color;
    output.normal = input.normal.rgb;

    output.objectSpaceLocation = mul(pushData->model, float4(input.location, 1.0)).xyz;

    return output;
}

[shader("fragment")]
float4 main(VSOutput input, uniform ProjectionViewModelUniform *pushData) : SV_Target
{
    float3 baseColor = diffuseMap.Sample(input.uv0).rgb;
    float3 ambient = (sceneLighting.ambientColor * sceneLighting.ambientStrength).rgb;

    float3 lightDir = normalize(LightDirection(lights[0], input.objectSpaceLocation));
    float3 normal = normalize(input.normal);

    float diff = max(dot(lightDir, normal), 0.0);
    float3 diffuse = baseColor * diff;

    return float4(ambient + diffuse, 1.0);
}