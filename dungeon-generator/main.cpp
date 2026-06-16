#include <Application.h>
#include <GameInstance.h>

class TestGameInstance : public GameInstance
{
public:
	void Init() override {}
	void Shutdown() override {}
	void Tick() override {}
	void Render() override {}
};

int main()
{
	return static_cast<int>(Application::Open<TestGameInstance>());
}