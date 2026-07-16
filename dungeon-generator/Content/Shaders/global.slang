struct VSInput
{
    float3 location;
    float4 normal;
    float4 tangent;
    float4 biTangent;

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
    float4x4 normal;
};

static const float PI = 3.14159265359f;

[[vk::binding(0, 0)]] ConstantBuffer<ProjectionViewUniform> projectionView;

float4 TransformVertex(float4x4 model, float3 vertex)
{
    return mul(projectionView->proj, mul(projectionView->view, mul(model, float4(vertex.xyz, 1.0))));
}

void CalculateNV(in float3 normal, in float3 worldLoc, out float3 N, out float3 V)
{
    N = normalize(normal);
    V = normalize(projectionView.cameraLocation - worldLoc);
}