#include <Application.h>
#include <GameInstance.h>

#include "Renderer.h"
#include "Rendering/Mesh.h"

class TestGameInstance : public GameInstance
{
public:
	void Init() override {}

	void Shutdown() override {}

	void Tick() override {}

	void Render(Renderer* renderer) override
	{
		renderer->RenderMesh(screenTriangleMesh);
	}
};

int main()
{
	return static_cast<int>(Application::Open<TestGameInstance>());
}