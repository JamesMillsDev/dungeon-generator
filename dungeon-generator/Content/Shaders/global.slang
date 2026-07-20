struct VSInput
{
    float4 location;
    float4 normal;
    float4 tangent;

    float2 uv0;
    float4 color;
};

struct VSOutput
{
    float4 location : SV_Position;
    float3 fragLocation;
    float3 normal;
    float4 tangent;
    
    float2 uv0;
    float4 color;
};

struct GlobalsUniform
{
    float4x4 proj;
    float4x4 view;
    float4x4 transform;
    float3 cameraLocation;

    float exposure;
    float gamma;
    float prefilteredCubeMipLevels;
    float scaleIBLAmbient;
};

static const float PI = 3.14159265359f;

[[vk::binding(0, 0)]] ConstantBuffer<GlobalsUniform> globals;

float4 TransformVertex(VSInput vertex)
{
    return mul(globals.proj, mul(globals.view, mul(globals.transform, vertex.location)));
}

float3 CalculateNormal(VSOutput vertex, Sampler2D normalMap, int32_t mapSet)
{
    // If normal map isn't available, return the normal
    if(mapSet == 0)
    {
        return normalize(vertex.normal);
    }

    // Sample normal map and convert from [0,1] to [-1,1] range
    float3 tangentNormal = normalMap.Sample(vertex.uv0).xyz * 2.0 - 1.0;

    // Construct tangent-space to world-space transformation matrix (TBN)
    float3 T = normalize(vertex.tangent.xyz);                            // Tangent
    float3 B = normalize(cross(vertex.normal, T)) * vertex.tangent.w;   // Bitangent (w = handedness)
    float3x3 TBN = float3x3(T, B, vertex.normal);                       // Tangent-Bitangent-Normal matrix

    return normalize(mul(tangentNormal, TBN));
}