#pragma once
#include <string>
#include <string_view>

class Window
{
protected:
	std::string WindowName;

public:
	Window(std::string Name) :
		WindowName(std::move(Name)) { }
	virtual void Draw() = 0;
	virtual ~Window() = default;

	std::string_view Name() const noexcept { return WindowName; }
};