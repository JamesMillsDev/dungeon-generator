import global;
import graphics;

struct PushConstants
{
    TransformUniform* transform;
    MaterialUniform* material;
};

[shader("vertex")]
VSOutput main(VSInput input, uniform PushConstants* pushConstants)
{
    VSOutput output;

    output.location = TransformVertex(input, pushConstants->transform->value);
    output.fragLocation = mul(pushConstants->transform->value, input.location).xyz;
    
    output.normal = input.normal.xyz;
    output.tangent = input.tangent;
    
    output.uv0 = input.uv0;
    output.color = input.color;

    return output;
}

[shader("fragment")]
float4 main(VSOutput input, uniform PushConstants* pushConstants) : SV_Target
{
    return pushConstants->material->color;
}