import global;
import graphics;

[shader("vertex")]
VSOutput main(VSInput input, uniform uint32_t objectIndex)
{
    VSOutput output;

    output.location = TransformVertex(input, objectIndex);
    output.fragLocation = mul(transforms[objectIndex].value, input.location).xyz;

    output.normal = input.normal.xyz;
    output.tangent = input.tangent;
    
    output.uv0 = input.uv0;
    output.color = input.color;

    return output;
}

[shader("fragment")]
float4 main(VSOutput input, uniform uint32_t objectIndex) : SV_Target
{
    return materials[objectIndex].color;
}