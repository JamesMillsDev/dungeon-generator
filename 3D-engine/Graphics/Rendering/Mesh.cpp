#include "pch.h"
#include "Mesh.h"

#include <tuple>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

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

Mesh* Mesh::Load(const string& file)
{
	string path = "./Content/" + file;

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(
		path.c_str(), 
		aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_GlobalScale
	);

	vector<Vertex> vertices;
	vector<uint16> indices;

	for (uint32 i = 0; i < scene->mNumMeshes; ++i)
	{
		const aiMesh* mesh = scene->mMeshes[i];
		vertices.resize(mesh->mNumVertices);

		for (uint32 v = 0; v < mesh->mNumVertices; ++v)
		{
			Vertex vert;

			if (mesh->HasPositions())
			{
				aiVector3D location = mesh->mVertices[v];

				vert.location = { location.x, location.y, location.z };
			}

			if (mesh->HasNormals())
			{
				aiVector3D normal = mesh->mNormals[v];

				vert.normal = { normal.x, normal.y, normal.z, 0.f };
			}

			if (mesh->HasTangentsAndBitangents())
			{
				aiVector3D tangent = mesh->mTangents[v];
				aiVector3D biTangent = mesh->mBitangents[v];

				vert.tangent = { tangent.x, tangent.y, tangent.z, 0.f };
				vert.biTangent = { biTangent.x, biTangent.y, biTangent.z, 0.f };
			}

			if (mesh->HasTextureCoords(0))
			{
				aiVector3D uv0 = mesh->mTextureCoords[0][v];

				vert.uv = { uv0.x, uv0.y };
			}

			if (mesh->HasVertexColors(0))
			{
				aiColor4D color0 = mesh->mColors[0][v];

				vert.color = Color{ color0.r, color0.g, color0.b, color0.a };
			}

			vertices[v] = vert;
		}

		if (mesh->HasFaces())
		{
			indices.reserve((size_t)(mesh->mNumFaces * 3));
			for (uint32 f = 0; f < mesh->mNumFaces; ++f)
			{
				aiFace face = mesh->mFaces[f];

				for (uint32 index = 0; index < face.mNumIndices; ++index)
				{
					indices.emplace_back(face.mIndices[index]);
				}
			}
		}
	}

	return new Mesh{ vertices, indices };
}

Mesh::Mesh(const vector<Vertex>& vertices, const vector<uint16>& indices)
	: vertices{ vertices }, indices{ indices }
{

}

void Mesh::CreateBuffers()
{
	
}

void Mesh::DestroyBuffers()
{
	
}

void Mesh::Render(const VkCommandBuffer buffer, const uint32 instances, const uint32 firstInstance) const
{
	VkBuffer vertexBuffers[] = { VK_NULL_HANDLE };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(buffer, 0, 1, vertexBuffers, offsets);

	vkCmdBindVertexBuffers(buffer, 0, 1, vertexBuffers, offsets);
	//vkCmdBindIndexBuffer(buffer, m_indexBuffer->Get(), 0, VK_INDEX_TYPE_UINT16);

	vkCmdDrawIndexed(
		buffer, static_cast<uint32>(indices.size()), instances, 0, 0, firstInstance
	);
}