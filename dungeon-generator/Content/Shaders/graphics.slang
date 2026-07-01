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