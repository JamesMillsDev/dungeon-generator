#include "global.slang"
#include "graphics.slang"

struct PushConstantData
{
    ProjectionViewUniform *ubo;
    MaterialUniform *material;
};

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
    float4 normal;
    float4 tangent;
    float4 biTangent;

    float2 uv0;
    float4 color;
};

[shader("vertex")]
VSOutput main(VSInput input, uniform PushConstantData *pushData)
{
    VSOutput output;

    output.location = mul(pushData->ubo->proj, mul(pushData->ubo->view, mul(pushData->ubo->model, float4(input.location.xyz, 1.0))));
    output.uv0 = input.uv0;
    output.normal = input.normal;
    output.tangent = input.tangent;
    output.biTangent = input.biTangent;
    output.color = input.color;

    return output;
}

[shader("fragment")]
float4 main(VSOutput input, uniform PushConstantData *pushData) : SV_Target
{
    return pushData->material->color;
}