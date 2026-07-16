struct MaterialUniform
{
    float4 color;
    float4 emissiveTint;

    float roughness;
    float metallic;
    float alphaMask;
    float alphaMaskCutoff;

    float exposure;
    float gamma;
    float prefilteredCubeMipLevels;
    float scaleIBLAmbient;

    int32_t baseColorMapSet;
    int32_t normalMapSet;
    int32_t ormMapSet;
    int32_t emissiveMapSet;
	int32_t heightMapSet;
};

[[vk::binding(3, 0)]] ConstantBuffer<MaterialUniform> material;
[[vk::binding(4, 0)]] ConstantBuffer<Sampler2D> diffuseMap;
[[vk::binding(5, 0)]] ConstantBuffer<Sampler2D> normalMap;
[[vk::binding(6, 0)]] ConstantBuffer<Sampler2D> ormMap;
[[vk::binding(7, 0)]] ConstantBuffer<Sampler2D> emissiveMap;
[[vk::binding(8, 0)]] ConstantBuffer<Sampler2D> heightMap;

float4 Sample(ConstantBuffer<Sampler2D> textures[], int32_t index, float2 uv, float4 defaultValue = float4(0, 0, 0, 0))
{
    if (index == -1)
    {
        return defaultValue;
    }

    return textures[index].Sample(uv);
}

float4 TrySampleMap(Sampler2D map, int32_t cond, float2 uv, float4 defaultValue)
{
    if(cond == 0)
    {
        return defaultValue;
    }

    return map.Sample(uv);
}