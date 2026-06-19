#include <Application.h>
#include <GameInstance.h>

#include "Renderer.h"
#include "Rendering/Mesh.h"
#include "Rendering/Texture.h"

class TestGameInstance : public GameInstance
{
public:
	void Init() override
	{
		m_mesh = Mesh::MakeQuad();
		Renderer::Load(m_mesh);
	}

	void Shutdown() override
	{
		Renderer::Unload(m_mesh);
	}

	void Tick() override
	{
		
	}

	void Render() override
	{
		m_renderer->Render(m_mesh);
	}

private:
	Mesh* m_mesh = nullptr;

};

int main()
{
	return static_cast<int>(Application::Open<TestGameInstance>());
}