struct LightUniform
{
	float4 location;
	float4 direction;
	float4 color;
	int32_t type;
};

float3 LightDirection(LightUniform* light, float3 location)
{
    if (light->type == 0)
    {
        return light->direction.xyz;
	}

    return normalize(light->location.xyz - location);
}