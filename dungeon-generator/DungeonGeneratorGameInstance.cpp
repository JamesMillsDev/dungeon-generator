#include "DungeonGeneratorGameInstance.h"

#include "Renderer.h"

#include "Actors/World.h"
#include "Actors/Components/Rendering/MeshComponent.h"

#include "Rendering/Mesh.h"

#include "VulkanHelpers/GraphicsPipeline.h"

DungeonGeneratorGameInstance::DungeonGeneratorGameInstance()
	: m_actor{ nullptr }, m_pipeline{ nullptr }
{
	
}

void DungeonGeneratorGameInstance::Init()
{
	m_pipeline = m_renderer->CreatePipeline(GraphicsPipelineConfig
		{
			vector<ShaderConfig>
			{
				{.stage = VK_SHADER_STAGE_VERTEX_BIT,   .shader = "Triangle.vert" },
				{.stage = VK_SHADER_STAGE_FRAGMENT_BIT, .shader = "Triangle.frag" }
			}
		});

	Mesh* mesh = Mesh::MakeQuad();

	m_actor = GetWorld()->MakeActor<Actor>();
	m_actor->MakeComponent<MeshComponent>(mesh, m_pipeline);
}

void DungeonGeneratorGameInstance::Shutdown() { }

void DungeonGeneratorGameInstance::Tick() { }

void DungeonGeneratorGameInstance::Render() { }