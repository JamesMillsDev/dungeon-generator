#include <Application.h>
#include "DungeonGeneratorGameInstance.h"

int main()
{
	return static_cast<int>(Application::Open<DungeonGeneratorGameInstance>());
}