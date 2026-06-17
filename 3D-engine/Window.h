#pragma once

#include <string>

using std::string;

struct GLFWwindow;

class Config;

class Window
{
	friend class Application;
	friend class Vulkan;

private:
	int m_width;
	int m_height;
	string m_title;
	bool m_resizable;
	bool m_isOpen;

	GLFWwindow* m_window;

private:
	explicit Window(Config* config);
	~Window();

public:
	[[nodiscard]] int Width() const;

	void SetWidth(int w);

	[[nodiscard]] int Height() const;

	void SetHeight(int h);

	[[nodiscard]] bool ShouldClose() const;

private:
	void Open();

	void Close();

};