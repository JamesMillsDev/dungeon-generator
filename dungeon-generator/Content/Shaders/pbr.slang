#include "global.slang"

struct VSInput
{
    float3 location;
    float4 normal;
    float4 tangent;
    float4 biTangent;

    float2 uv0;
    float4 color;
};

struct VSOutput
{
    float4 location : SV_Position;
    float3 normal;
    float2 uv0;
};

[[vk::push_constant]] float4x4 transform;

[shader("vertex")]
VSOutput main(VSInput input, uint instanceIndex: SV_VulkanInstanceID)
{
    VSOutput output;

    output.location = mul(ubo.proj, mul(ubo.view, mul(transform, float4(input.location.xyz, 1.0))));
    output.uv0 = input.uv0;

    return output;
}

[shader("fragment")]
float4 main(VSOutput input, uniform MaterialUniform *material)
{
    return float4(1.0);
}