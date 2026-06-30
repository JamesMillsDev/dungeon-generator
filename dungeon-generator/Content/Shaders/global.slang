struct ProjectionViewUniform
{
    float4x4 proj;
    float4x4 view;
    float4x4 model;
    float3 cameraLocation;
};