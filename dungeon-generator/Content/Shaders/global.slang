struct ProjectionViewUniform
{
    float4x4 proj;
    float4x4 view;
    float3 cameraLocation;
};

ConstantBuffer<ProjectionViewUniform> ubo;

[[vk::push_constant]] float4x4 transform;