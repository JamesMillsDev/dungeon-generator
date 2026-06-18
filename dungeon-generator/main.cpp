#include <Application.h>
#include <GameInstance.h>

#include "Renderer.h"
#include "Rendering/Mesh.h"

class TestGameInstance : public GameInstance
{
public:
	void Init() override
	{
		m_mesh = Mesh::MakeQuad();
		m_renderer->LoadMesh(m_mesh);
	}

	void Shutdown() override
	{
		m_renderer->UnloadMesh(m_mesh);
	}

	void Tick() override {}

	void Render() override
	{
		m_renderer->RenderMesh(m_mesh);
	}

private:
	Mesh* m_mesh = nullptr;

};

int main()
{
	return static_cast<int>(Application::Open<TestGameInstance>());
}