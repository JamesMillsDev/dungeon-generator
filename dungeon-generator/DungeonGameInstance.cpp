#include "DungeonGameInstance.h"

#include "Renderer.h"

#include "Actors/World.h"
#include "Actors/Components/Rendering/MeshComponent.h"

#include "Rendering/Mesh.h"

#include "VulkanHelpers/GraphicsPipeline.h"

DungeonGameInstance::DungeonGameInstance()
	: m_actor{ nullptr }, m_pipeline{ nullptr }
{
	
}

void DungeonGameInstance::Init()
{
	m_pipeline = m_renderer->CreatePipeline(GraphicsPipelineConfig
		{
			ShaderConfig
			{
				.stages = static_cast<VkShaderStageFlagBits>(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT),
				.passCount = 2,
				.name = "Triangle"
			}
		});

	Mesh* mesh = Mesh::MakeQuad();

	m_actor = GetWorld()->MakeActor<Actor>();
	m_actor->MakeComponent<MeshComponent>(mesh, m_pipeline);
}

void DungeonGameInstance::Shutdown() { }

void DungeonGameInstance::Tick() { }

void DungeonGameInstance::Render() { }