#pragma once

#include <array>
#include <vector>

#include <vulkan/vulkan.h>

#include "Maths/Color.h"
#include "Maths/Vector2.h"
#include "Maths/Vector3.h"
#include "Maths/Vector4.h"

class Buffer;

using std::array;
using std::vector;

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
	Vector3 location;
	/** @brief The normal of the vertex in model space. */
	Vector4 normal;
	/** @brief The tangent of the vertex in model space. */
	Vector4 tangent;
	/** @brief The bitangent of the vertex in model space. */
	Vector4 biTangent;

	/** @brief The first texture coordinate of the vertex. */
	Vector2 uv;
	/** @brief The first color of the vertex. */
	Color color;

};

class Mesh
{
	friend class Renderer;
	friend class Vulkan;

public:
	static Mesh* MakeQuad();
	static Mesh* Load(const string& file);

public:
	vector<Vertex> vertices;
	vector<uint16> indices;

private:
	Buffer* m_vertexBuffer;
	Buffer* m_indexBuffer;

public:
	Mesh(const vector<Vertex>& vertices, const vector<uint16>& indices);

private:
	void CreateBuffers();
	void DestroyBuffers();

	void Render(VkCommandBuffer buffer, uint32 instances = 1, uint32 firstInstance = 0) const;

};

extern Mesh* screenTriangleMesh;