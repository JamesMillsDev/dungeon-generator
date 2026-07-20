import global;
import graphics;

[shader("vertex")]
VSOutput main(VSInput input, uniform MaterialUniform material)
{
    VSOutput output;

    output.location = TransformVertex(input);
    output.fragLocation = mul(globals.transform, input.location).xyz;

    output.normal = input.normal.xyz;
    output.tangent = input.tangent;
    
    output.uv0 = input.uv0;
    output.color = input.color;

    return output;
}

[shader("fragment")]
float4 main(VSOutput input, uniform MaterialUniform material) : SV_Target
{
    return material.color;
}