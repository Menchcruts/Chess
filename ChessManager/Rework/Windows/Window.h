#pragma once

class Window
{
protected:
	const char* WindowName;

public:
	Window(const char* Name) :
		WindowName(Name) { }
	virtual void Draw() = 0;
	virtual ~Window() = default;
};