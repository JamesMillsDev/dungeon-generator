#pragma once

#include <array>
#include <vector>
#include <vma/vk_mem_alloc.h>

#include <vulkan/vulkan.h>

#include "Maths/Color.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

class VulkanBuffer;

using std::array;
using std::vector;
using glm::vec2;
using glm::vec3;
using glm::vec4;

enum : uint8
{
	LocationIndex,
	NormalIndex,
	TangentIndex,
	BiTangentIndex,
	UvIndex,
	ColorIndex,
	VertexAttributeCount
};

struct Vertex
{
public:
	static VkVertexInputBindingDescription GetBindingDescription();
	static array<VkVertexInputAttributeDescription, VertexAttributeCount> GetAttributeDescriptions();

public:
	/** @brief The location of the vertex in model space. */
	vec3 location;
	/** @brief The normal of the vertex in model space. */
	vec4 normal;
	/** @brief The tangent of the vertex in model space. */
	vec4 tangent;
	/** @brief The bitangent of the vertex in model space. */
	vec4 biTangent;

	/** @brief The first texture coordinate of the vertex. */
	vec2 uv;
	/** @brief The first color of the vertex. */
	Color color;

};

class Mesh
{
	friend class Renderer;
	friend class Vulkan;

public:
	struct SubMesh
	{
		friend Mesh;

	public:
		vector<Vertex> vertices;
		vector<uint16> indices;

	private:
		VkDeviceSize m_vertexBufferSize;
		VkDeviceSize m_indexBufferSize;

		VulkanBuffer* m_vertexBuffer;

	public:
		SubMesh(const vector<Vertex>& vertices, const vector<uint16>& indices);
		~SubMesh();

	private:
		void CreateBuffer();

	};

public:
	static Mesh* MakeQuad();
	static Mesh* MakeFromAssimp(const string& file);

public:
	vector<SubMesh*> subMeshes;

public:
	explicit Mesh(const vector<SubMesh*>& subMeshes);
	~Mesh();

private:
	void CreateBuffers();
	void DestroyBuffers();

	void Render(VkCommandBuffer buffer, uint32 instances = 1, uint32 firstInstance = 0) const;

};