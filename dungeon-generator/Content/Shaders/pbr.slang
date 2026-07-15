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

    int32_t baseColorMap;
    int32_t normalMap;
    int32_t ormMap;
    int32_t emissiveMap;
};

[[vk::binding(2, 0)]] ConstantBuffer<MaterialUniform> material;
[[vk::binding(3, 0)]] ConstantBuffer<Sampler2D> diffuseMap;
[[vk::binding(4, 0)]] ConstantBuffer<Sampler2D> normalMap;
[[vk::binding(5, 0)]] ConstantBuffer<Sampler2D> ormMap;

struct VSOutput
{
    float4 location : SV_Position;
    float3x3 tbn;

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

    float3 t = normalize(mul(pushData->model, input.tangent).xyz);
    float3 b = normalize(mul(pushData->model, input.biTangent).xyz);
    float3 n = normalize(mul(pushData->model, input.normal).xyz);
    output.tbn = float3x3(t, b, n);

    output.color = input.color;

    output.objectSpaceLocation = mul(pushData->model, float4(input.location, 1.0)).xyz;

    return output;
}

[shader("fragment")]
float4 main(VSOutput input, uniform ProjectionViewModelUniform *pushData) : SV_Target
{
    float3 normal = MapNormal(normalMap, input.uv0);
    normal = normalize(mul(input.tbn, normal));

    float diff = max(dot(normal, LightDirection(lights[0], input.objectSpaceLocation)), 0);

    float3 viewDir = normalize(pushData->cameraLocation - input.objectSpaceLocation);
    float3 reflectDir = reflect(-LightDirection(lights[0], input.objectSpaceLocation), normal);

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    float4 specular = material.specularStrength * spec * material.specularColor;

    float4 ambient = sceneLighting.ambientColor * sceneLighting.ambientStrength;
    float4 diffuse = lights[0].color * diff;

    float4 textureColor = diffuseMap.Sample(input.uv0);

    float4 result = saturate((ambient + diff + specular) * textureColor);

    return result;
}