#include "pch.h"
#include "Mesh.h"

#include <tuple>

#include "VulkanHelpers/Buffer.h"
#include "VulkanHelpers/Vulkan.h"

using std::vector;
using VertexAttribData = std::tuple<uint8, uint8, VkFormat, size_t>;

const vector<VertexAttribData> VERTEX_ATTRIBUTES =
{
	std::make_tuple(LocationIndex, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, location)),
	std::make_tuple(NormalIndex, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)),
	std::make_tuple(TangentIndex, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, tangent)),
	std::make_tuple(BiTangentIndex, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, biTangent)),
	std::make_tuple(UvIndex, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)),
	std::make_tuple(ColorIndex, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, color))
};

VkVertexInputBindingDescription Vertex::GetBindingDescription()
{
	VkVertexInputBindingDescription bindingDescription;

	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

array<VkVertexInputAttributeDescription, VertexAttributeCount> Vertex::GetAttributeDescriptions()
{
	array<VkVertexInputAttributeDescription, VertexAttributeCount> attributeDescriptions;

	// Iterate over each attribute definition and assign the descriptor
	for (const auto& [index, binding, format, offset] : VERTEX_ATTRIBUTES)
	{
		attributeDescriptions[index].binding = binding;
		attributeDescriptions[index].location = index;
		attributeDescriptions[index].format = format;
		attributeDescriptions[index].offset = static_cast<uint32>(offset);
	}

	return attributeDescriptions;
}

Mesh* Mesh::MakeQuad()
{
	return new Mesh
	{
		{
			{
				.location = { -.5f, -.5f, 0.f },
				.normal = { 0.f, 0.f, 0.f, 0.f },
				.tangent = { 0.f, 0.f, 0.f, 0.f },
				.biTangent = { 0.f, 0.f, 0.f, 0.f },
				.uv = { 1.f, 0.f },
				.color = { 1.f, 0.f, 0.f, 1.f }
			},
			{
				.location = { .5f, -.5f, 0.f },
				.normal = { 0.f, 0.f, 0.f, 0.f },
				.tangent = { 0.f, 0.f, 0.f, 0.f },
				.biTangent = { 0.f, 0.f, 0.f, 0.f },
				.uv = { 0.f, 0.f },
				.color = { 0.f, 1.f, 0.0f, 1.f }
			},
			{
				.location = { .5f, .5f, 0.f },
				.normal = { 0.f, 0.f, 0.f, 0.f },
				.tangent = { 0.f, 0.f, 0.f, 0.f },
				.biTangent = { 0.f, 0.f, 0.f, 0.f },
				.uv = { 0.f, 1.f },
				.color = { 0.f, 0.f, 1.f, 1.f }
			},
			{
				.location = { -.5f, .5f, 0.f },
				.normal = { 0.f, 0.f, 0.f, 0.f },
				.tangent = { 0.f, 0.f, 0.f, 0.f },
				.biTangent = { 0.f, 0.f, 0.f, 0.f },
				.uv = { 1.f, 1.f },
				.color = { 1.f, 1.f, 1.f, 1.f }
			}
		},
		{
			0, 1, 2, 2, 3, 0
		}
	};
}

Mesh::Mesh(const vector<Vertex>& vertices, const vector<uint16>& indices)
	: vertices{ vertices }, indices{ indices }, m_vertexBuffer{ nullptr }, m_indexBuffer{ nullptr }
{

}

void Mesh::CreateBuffers()
{
	Vulkan* vulkan = Vulkan::Instance();

	Buffer* stagingBuffer = vulkan->MakeStagingBuffer(sizeof(Vertex), vertices.size());
	stagingBuffer->Fill(vertices.data());

	m_vertexBuffer = vulkan->MakeVertexBuffer(vertices.size());
	m_vertexBuffer->Copy(stagingBuffer, stagingBuffer->Size());

	Vulkan::DestroyBuffer(stagingBuffer);

	stagingBuffer = vulkan->MakeStagingBuffer(sizeof(uint16), indices.size());
	stagingBuffer->Fill(indices.data());

	m_indexBuffer = vulkan->MakeIndexBuffer(indices.size());
	m_indexBuffer->Copy(stagingBuffer, stagingBuffer->Size());

	Vulkan::DestroyBuffer(stagingBuffer);
}

void Mesh::DestroyBuffers()
{
	Vulkan::DestroyBuffer(m_vertexBuffer);
	Vulkan::DestroyBuffer(m_indexBuffer);
}

void Mesh::Render(const VkCommandBuffer buffer, const uint32 instances, const uint32 firstInstance) const
{
	VkBuffer vertexBuffers[] = { m_vertexBuffer->Get() };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(buffer, 0, 1, vertexBuffers, offsets);

	vkCmdBindVertexBuffers(buffer, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(buffer, m_indexBuffer->Get(), 0, VK_INDEX_TYPE_UINT16);

	vkCmdDrawIndexed(
		buffer, static_cast<uint32>(indices.size()), instances, 0, 0, firstInstance
	);
}

Mesh* screenTriangleMesh;
