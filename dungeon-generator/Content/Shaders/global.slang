struct VSInput
{
    float3 location;
    float4 normal;
    float4 tangent;

    float2 uv0;
    float4 color;
};

struct VSOutput
{
    float4 location : SV_Position;
    float3 normal;
    float4 tangent;
    
    float2 uv0;
    float4 color;
};

struct ProjectionViewUniform
{
    float4x4 proj;
    float4x4 view;
    float3 cameraLocation;
};

struct TransformUniform
{
    float4x4 model;
};

static const float PI = 3.14159265359f;

[[vk::binding(0, 0)]] ConstantBuffer<ProjectionViewUniform> projectionView;

float4 TransformVertex(float4x4 model, float3 vertex)
{
    return mul(projectionView.proj, mul(projectionView.view, mul(model, float4(vertex.xyz, 1.0))));
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