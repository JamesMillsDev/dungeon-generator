struct MaterialUniform
{
    float4 color;
    float4 emissiveTint;
    float roughness;
    float metallic;

    int32_t baseColorMap;
    int32_t normalMap;
    int32_t ormMap;
    int32_t emissiveMap;
};

[[vk::binding(2, 0)]]Sampler2D textures[];

float4 Sample(int32_t index, float2 uv, float4 defaultValue = float4(0, 0, 0, 0))
{
    if (index == -1)
    {
        return defaultValue;
    }

    return textures[index].Sample(uv);
}