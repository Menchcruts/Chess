#pragma once
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <vector>
#include <memory>
#include <concepts>

#include "Utils/Logger.h"
#include "Windows/Window.h"
#include "Chess/Chessboard_new.h"

template<typename _Window>
concept WindowType = std::derived_from<_Window, Window>;

template<typename _Window, typename... Args>
concept CtorMatches = WindowType<_Window> && std::constructible_from<_Window, Args...>;

namespace assets { class ImageManager; }

class App
{
public:
	App();
	~App();
	void Run();

private:
	std::vector<std::unique_ptr<Window>> m_Windows;
	Logger m_Logger;

	std::unique_ptr<assets::ImageManager> m_Images;
	std::unique_ptr<Chess_Rework::Chessboard_New> m_Board;
	GLFWwindow* m_Window;

private:
	void PreFrame();
	void PostFrame();
	void Draw();
	void SetupDockspace();

	void LoadImages();

	template<class _Window, typename... Args> requires WindowType<_Window>
	_Window& AddWindow(std::string name, Args&&... args)
	{
		std::string label = name + "##" + std::to_string(m_Windows.size());
		
		auto emplace = [this, label = std::move(label)](auto&&... xs) mutable -> _Window&
			{
				auto up = std::make_unique<_Window>(std::move(label), std::forward<decltype(xs)>(xs)...);
				_Window& ref = *up;
				m_Windows.emplace_back(std::move(up));
				return ref;
			};

		if constexpr (CtorMatches<_Window, Args...>)
		{
			return emplace(std::forward<Args>(args)...);
		}
		// Optional: DI branch for ImageManager& if your windows expect it.
		else if constexpr (requires { m_Images.get(); } && std::constructible_from<_Window, std::string, Args..., assets::ImageManager&>)
		{
			if (!m_Images) throw std::logic_error("AddWindow: ImageManager not initialized.");
			return emplace(std::forward<Args>(args)..., *m_Images);
		}
		// Optional: DI branch for App&
		else if constexpr (std::constructible_from<_Window, std::string, Args..., App&>)
		{
			return emplace(std::forward<Args>(args)..., *this);
		}
		else
		{
			static_assert([] { return false; }(),
				"AddWindow: constructor not matched (even with DI of ImageManager/App).");
		}
	}
};