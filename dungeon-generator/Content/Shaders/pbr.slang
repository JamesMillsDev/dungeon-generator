#include "global.slang"
#include "graphics.slang"
#include "lighting.slang"

struct PushConstantData
{
    ProjectionViewModelUniform *pvm;
    MaterialUniform *material;
    LightUniform *lighting;
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
    float3 normal;
    float4 tangent;
    float4 biTangent;

    float2 uv0;
    float4 color;
    float3 objectSpaceLocation;
};

[shader("vertex")]
VSOutput main(VSInput input, uniform PushConstantData *pushData)
{
    VSOutput output;

    output.location = TransformVertex(pushData->pvm, input.location);
    output.uv0 = input.uv0;
    output.normal = normalize(input.normal).xyz;
    output.tangent = normalize(input.tangent);
    output.biTangent = normalize(input.biTangent);
    output.color = input.color;

    output.objectSpaceLocation = mul(pushData->pvm->model, float4(input.location, 1.0)).xyz;

    return output;
}

[shader("fragment")]
float4 main(VSOutput input, uniform PushConstantData *pushData) : SV_Target
{
    float diff = max(dot(input.normal, LightDirection(pushData->lighting, input.objectSpaceLocation)), 0);

    float4 color = Sample(pushData->material->baseColorMap, input.uv0, float4(1.0, 1.0, 1.0, 1.0)) * pushData->material->color;

    return diff * color;
}