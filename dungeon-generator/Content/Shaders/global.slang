struct ProjectionViewUniform
{
    float4x4 proj;
    float4x4 view;
    float4x4 model;
    float3 cameraLocation;
};

float4 TransformVertex(ProjectionViewUniform* ubo, float3 vertex)
{
    return mul(ubo->proj, mul(ubo->view, mul(ubo->model, float4(vertex.xyz, 1.0))));
}