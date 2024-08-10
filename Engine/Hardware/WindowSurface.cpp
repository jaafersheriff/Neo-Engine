#include "Hardware/pch.hpp"

#include "WindowSurface.hpp"
#include "Mouse.hpp"
#include "Keyboard.hpp"

#include "Renderer/Renderer.hpp"
#include "Engine/ImGuiManager.hpp"
#include "Messaging/Messenger.hpp"

#include "Util/Profiler.hpp"
#include "Util/RenderThread.hpp"
#include "Util/ServiceLocator.hpp"
#include <GLFW/glfw3.h>

namespace neo {

	namespace {

		static void _errorCallback(int error, const char* desc) {
			NEO_FAIL("GLFW Error %d: %s", error, desc);
		}

	}
	int WindowSurface::init(const std::string& name) {
		/* Set error callback */
		glfwSetErrorCallback(_errorCallback);

		/* Init GLFW */
		NEO_ASSERT(glfwInit(), "Error initializing GLFW");

		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
		glfwWindowHint(GLFW_DEPTH_BITS, 0);
		glfwWindowHint(GLFW_STENCIL_BITS, 0);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, ServiceLocator<Renderer>::ref().mDetails.mGLMajorVersion);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, ServiceLocator<Renderer>::ref().mDetails.mGLMinorVersion);
		glfwWindowHint(GLFW_AUTO_ICONIFY, false);
		glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

#ifdef DEBUG_MODE
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

		const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		NEO_ASSERT(mode, "glfwGetVideoMode failed");
		mDetails.mRefreshRate = mode->refreshRate;

		{
			float x, y;
			glfwGetMonitorContentScale(glfwGetPrimaryMonitor(), &x, &y);
			mDetails.mDPIScale = x;
		}

		/* Create GLFW window */
		mWindow = glfwCreateWindow(mDetails.mSize.x, mDetails.mSize.y, name.c_str(), NULL, NULL);
		if (!mWindow) {
			glfwTerminate();
			NEO_FAIL("Failed to create window");
		}
		glfwSetWindowUserPointer(mWindow, &mDetails);

		glfwSetKeyCallback(mWindow, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
			// These should definitely move elsewhere lol
			auto& imguiManager = ServiceLocator<ImGuiManager>::ref();
			if ((key == GLFW_KEY_F11 || key == GLFW_KEY_ENTER && mods & GLFW_MOD_ALT) && action == GLFW_PRESS) {
				Messenger::sendMessage<ToggleFullscreenMessage>(glfwGetWindowMonitor(window) != nullptr);
				return;
			}
			if (key == GLFW_KEY_GRAVE_ACCENT && action == GLFW_PRESS) {
				imguiManager.toggleImGui();
				if (!imguiManager.isEnabled()) {
					WindowDetails& details = *(WindowDetails*)glfwGetWindowUserPointer(window);
					int x, y;
					glfwGetFramebufferSize(window, &x, &y);
					details.mSize.x = x;
					details.mSize.y = y;
					Messenger::sendMessage<FrameSizeMessage>(details.mSize);
				}
			}
			if (imguiManager.isEnabled()) {
				ServiceLocator<ImGuiManager>::ref().updateKeyboard(window, key, scancode, action, mods);
			}
			if (!imguiManager.isEnabled() || imguiManager.isViewportFocused()) {
				Messenger::sendMessage<Keyboard::KeyPressedMessage>(key, action);
			}
			else {
				Messenger::sendMessage<Keyboard::ResetKeyboardMessage>();
			}
		});

		glfwSetMouseButtonCallback(mWindow, [](GLFWwindow* window, int button, int action, int mods) {
			auto& imguiManager = ServiceLocator<ImGuiManager>::ref();
			if (imguiManager.isEnabled()) {
				imguiManager.updateMouse(window, button, action, mods);
			}
			if (!imguiManager.isEnabled() || imguiManager.isViewportHovered()) {
				Messenger::sendMessage<Mouse::MouseButtonMessage>(button, action);
				if (action == GLFW_PRESS) {
					ImGui::SetWindowFocus("Viewport");
				}
			}
			else {
				Messenger::sendMessage<Mouse::MouseResetMessage>();
			}
		});

		glfwSetScrollCallback(mWindow, [](GLFWwindow* window, double dx, double dy) {
			auto& imguiManager = ServiceLocator<ImGuiManager>::ref();
			if (imguiManager.isEnabled()) {
				imguiManager.updateScroll(window, dx, dy);
			}
			if (!imguiManager.isEnabled() || imguiManager.isViewportHovered()) {
				Messenger::sendMessage<Mouse::ScrollWheelMessage>(dy);
			}
			else {
				Messenger::sendMessage<Mouse::MouseResetMessage>();
			}
			});
		glfwSetCharCallback(mWindow, [](GLFWwindow* window, unsigned int c) {
			if (ServiceLocator<ImGuiManager>::ref().isEnabled() && !ServiceLocator<ImGuiManager>::ref().isViewportFocused()) {
				ServiceLocator<ImGuiManager>::ref().updateCharacter(window, c);
			}
			});

		glfwSetWindowSizeCallback(mWindow, [](GLFWwindow* window, int width, int height) {
			NEO_UNUSED(window);
			if (width == 0 || height == 0) {
				return;
			}

			Messenger::sendMessage<FrameSizeMessage>(glm::uvec2(width, height));
			});

		glfwSetFramebufferSizeCallback(mWindow, [](GLFWwindow* window, int width, int height) {
			NEO_UNUSED(window);
			if (width == 0 || height == 0) {
				return;
			}

			Messenger::sendMessage<FrameSizeMessage>(glm::uvec2(width, height));
			});

		glfwSetCursorEnterCallback(mWindow, [](GLFWwindow* window, int entered) {
			NEO_UNUSED(window, entered);
			Messenger::sendMessage<Mouse::MouseResetMessage>();
		});

		reset(name);

		return 0;
	}

	void WindowSurface::_onFrameSizeChanged(const FrameSizeMessage& msg) {
		const FrameSizeMessage& m(static_cast<const FrameSizeMessage&>(msg));

		mDetails.mSize.x = m.mSize.x;
		mDetails.mSize.y = m.mSize.y;
	}

	void WindowSurface::_onToggleFullscreen(const ToggleFullscreenMessage& msg) {
		const ToggleFullscreenMessage& m(static_cast<const ToggleFullscreenMessage&>(msg));
		/* If already full screen */
		if (m.mAlreadyFullscreen) {
			mDetails.mFullscreen = false;
			mDetails.mSize = { 1920, 1080 };
			glfwSetWindowMonitor(mWindow, nullptr, mDetails.mPos.x, mDetails.mPos.y, mDetails.mSize.x, mDetails.mSize.y, mDetails.mVSyncEnabled);
		}
		/* If windowed */
		else {
			int x, y;
			glfwGetWindowPos(mWindow, &x, &y);
			mDetails.mPos.x = x;
			mDetails.mPos.y = y;
			mDetails.mFullscreen = true;
			GLFWmonitor* monitor(glfwGetPrimaryMonitor());
			const GLFWvidmode* video(glfwGetVideoMode(monitor));
			glfwSetWindowMonitor(mWindow, monitor, 0, 0, video->width, video->height, video->refreshRate);
		}
	}

	void WindowSurface::reset(const std::string& name) {
		glfwSetWindowTitle(mWindow, name.c_str());

		/* Set callbacks */
		Messenger::removeReceiver<ToggleFullscreenMessage>(this);
		Messenger::removeReceiver<FrameSizeMessage>(this);
		Messenger::addReceiver<ToggleFullscreenMessage, &WindowSurface::_onToggleFullscreen>(this);
		Messenger::addReceiver<FrameSizeMessage, &WindowSurface::_onFrameSizeChanged>(this);

		if (!ServiceLocator<ImGuiManager>::empty()) {
			if (!ServiceLocator<ImGuiManager>::ref().isEnabled()) {
				int x, y;
				glfwGetFramebufferSize(mWindow, &x, &y);
				mDetails.mSize.x = x;
				mDetails.mSize.y = y;
				Messenger::sendMessage<FrameSizeMessage>(mDetails.mSize);
			}
		}
	}

	void WindowSurface::updateHardware() {
		TRACY_ZONE();
		if (!ServiceLocator<ImGuiManager>::ref().isEnabled()) {
			double x, y;
			glfwGetCursorPos(mWindow, &x, &y);
			y = mDetails.mSize.y - y;
			Messenger::sendMessage<Mouse::MouseMoveMessage>(x, y);
		}
		Messenger::sendMessage<Mouse::ScrollWheelMessage>(0.0);
		{
			TRACY_ZONEN("glfwPollEvents");
			glfwPollEvents();
		}
	}

	void WindowSurface::setSize(const glm::ivec2& size) {
		if (!mDetails.mFullscreen) {
			mDetails.mSize.x = size.x;
			mDetails.mSize.y = size.y;
			glfwSetWindowSize(mWindow, mDetails.mSize.x, mDetails.mSize.y);
		}
	}

	void WindowSurface::toggleVSync() {
		mDetails.mVSyncEnabled = !mDetails.mVSyncEnabled;
		_setVsync();
	}

	int WindowSurface::shouldClose() const {
		return glfwWindowShouldClose(mWindow);
	}

	int WindowSurface::isMinimized() const {
		return glfwGetWindowAttrib(mWindow, GLFW_ICONIFIED);
	}

	void WindowSurface::flip() {
		TRACY_ZONE();
		glfwSwapBuffers(mWindow);
	}

	void WindowSurface::shutDown() {
		/* Clean up GLFW */
		glfwDestroyWindow(mWindow);
		glfwTerminate();
	}

	void WindowSurface::_setVsync() {
		ServiceLocator<RenderThread>::ref().pushRenderFunc([vsync = mDetails.mVSyncEnabled]() {
			glfwSwapInterval(vsync);
		});
	}
}