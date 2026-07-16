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

    float3 fragPos;
    float3x3 tbn;
    
    float2 uv0;
    float4 color;
};

[shader("vertex")]
VSOutput main(VSInput input, uniform TransformUniform *pushData)
{
    VSOutput output;

    output.location = TransformVertex(pushData->model, input.location);
    output.uv0 = input.uv0;

    float4 normal = MapNormals(normalMap, input.uv0);
    output.tbn = transpose(CalculateTBN(normal, input.tangent, input.biTangent, pushData->normal));
    output.fragPos = mul(pushData->model, float4(input.location, 1.0)).xyz;

    output.color = input.color;

    return output;
}

[shader("fragment")]
float4 main(VSOutput input, uniform TransformUniform *pushData) : SV_Target
{
    float4 baseColor = diffuseMap.Sample(input.uv0) * material.color;
    float4 orm = ormMap.Sample(input.uv0);
    //float4 baseColor = material.color;

    float4 sceneAmbient = sceneLighting.ambientColor * sceneLighting.ambientStrength;

    float4 Lo = CalculateRadiance(input.location.xyz, input.tbn, orm, baseColor);

    float4 ambient = sceneAmbient * baseColor * orm.r;

    float4 color = ambient + Lo;
    color = color / (color + float4(1.0));
    color = pow(color, float4(1.0 / 2.2));

    return color;
}