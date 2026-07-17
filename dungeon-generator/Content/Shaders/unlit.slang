import global;
import graphics;

[shader("vertex")]
VSOutput main(VSInput input, uniform float4x4 transform)
{
    VSOutput output;

    output.location = TransformVertex(input, transform);
    output.fragLocation = mul(transform, input.location).xyz;
    
    output.normal = input.normal.xyz;
    output.tangent = input.tangent;
    
    output.uv0 = input.uv0;
    output.color = input.color;

    return output;
}

[shader("fragment")]
float4 main(VSOutput input, uniform float4x4 transform) : SV_Target
{
    return material.color;
}