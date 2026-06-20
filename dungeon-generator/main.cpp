#include <Application.h>
#include <GameInstance.h>

#include "Renderer.h"

#include "Actors/Actor.h"
#include "Actors/World.h"
#include "Actors/Components/Rendering/MeshComponent.h"

#include "Rendering/Mesh.h"

class TestGameInstance : public GameInstance
{
public:
	void Init() override
	{
		Mesh* mesh = Mesh::MakeQuad();

		m_actor = GetWorld()->MakeActor<Actor>();
		MeshComponent* comp = m_actor->MakeComponent<MeshComponent>(mesh);
	}

	void Shutdown() override
	{

	}

	void Tick() override
	{
		
	}

	void Render() override
	{

	}

private:
	Actor* m_actor = nullptr;

};

int main()
{
	return static_cast<int>(Application::Open<TestGameInstance>());
}