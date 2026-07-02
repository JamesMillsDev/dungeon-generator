struct ProjectionViewModelUniform
{
    float4x4 proj;
    float4x4 view;
    float4x4 model;
    float3 cameraLocation;
};

float4 TransformVertex(ProjectionViewModelUniform* pvm, float3 vertex)
{
    return mul(pvm->proj, mul(pvm->view, mul(pvm->model, float4(vertex.xyz, 1.0))));
}