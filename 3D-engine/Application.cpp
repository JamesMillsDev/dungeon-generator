#include "pch.h"
#include "Application.h"

#include <iostream>
#include <stdexcept>
#include <GLFW/glfw3.h>

#include "GameTime.h"
#include "Renderer.h"

#include "Utility/Config.h"

using std::runtime_error;

Application* Application::m_instance = nullptr;

Application* Application::Instance()
{
	return m_instance;
}

void Application::Quit()
{
	m_instance->m_window->m_isOpen = false;
}

Application::Application()
	: m_config{ new Config{ "Engine" } }, m_window{ new Window{ m_config } }, m_game{ nullptr },
	m_renderer{ nullptr }
{}

Application::~Application()
{
	delete m_renderer;
	m_renderer = nullptr;

	delete m_game;
	m_game = nullptr;

	delete m_window;
	m_window = nullptr;

	delete m_config;
	m_config = nullptr;
}

EExitCode Application::Run()
{
	// Attempt to open the window, returning fail code if it does not succeed
	try
	{
		m_window->Open();
	}
	catch (runtime_error& e)
	{
		std::cout << e.what() << "\n";
		return EExitCode::WindowFailedToOpen;
	}

	m_renderer = new Renderer{ m_window->m_window, m_config };
	m_renderer->Create();
	// Validate the renderer succeeded to initialise
	if (!m_renderer->IsValid())
	{
		// It didn't, so shutdown the window and return the error code.
		m_window->Close();
		return EExitCode::RendererFailedToInit;
	}

	GameTime::Init();

	// Initialise the game instance
	m_game->m_renderer = m_renderer;
	m_game->Init();

	// Continue to loop until the window requests a close
	while (!m_window->ShouldClose())
	{
		GameTime::Tick();

		glfwPollEvents();

		m_game->Tick();

		m_renderer->BeginFrame();

		m_game->Render();

		m_renderer->EndFrame();
	}

	m_renderer->WaitDeviceIdle();

	// Shutdown the game instance and close the window
	m_game->Shutdown();
	m_renderer->Destroy();
	m_window->Close();

	// Return success as the whole gameplay loop ran successfully.
	return EExitCode::Success;
}
