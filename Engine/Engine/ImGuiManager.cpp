#include "ImGuiManager.hpp"

#include "Messaging/Messenger.hpp"

#include "ECS/Component/RenderingComponent/ImGuiDrawComponent.hpp"

#include "ResourceManager/ResourceManagers.hpp"

#include "Engine/Engine.hpp"
#include "Renderer/Renderer.hpp"
#include "Hardware/WindowSurface.hpp"
#include "Hardware/Mouse.hpp"

#include "Util/Util.hpp"
#include "Util/Profiler.hpp"
#include "Util/Log/Log.hpp"
#include "Util/ServiceLocator.hpp"

#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>
#include <ext/implot/implot.h>
#include <ImGuizmo.h>

#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>

namespace {
	static void weirdImGuiDrawData(ImGuiViewport*, void*) {
		NEO_FAIL("HEH");
	}
}

namespace neo {

	void ImGuiManager::init(GLFWwindow* window, float dpiScale) {
		/* Init ImGui */
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImPlot::CreateContext();
		ImGuizmo::Enable(true);
		ImGuizmo::SetOrthographic(false);
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.FontGlobalScale = dpiScale;

		ImGuiStyle* style = &ImGui::GetStyle();
		style->ScaleAllSizes(io.FontGlobalScale);
		ImVec4* colors = style->Colors;
#define HEXTOIM(x) x >= 0x1000000 ? ImVec4( ((x >> 24) & 0xFF) /255.f, ((x >> 16) & 0xFF)/255.f, ((x >> 8) & 0xFF)/255.f, ((x) & 0xFF)/255.f): ImVec4( ((x >> 16) & 0xFF) /255.f, ((x >> 8) & 0xFF)/255.f, ((x) & 0xFF)/255.f, 1.f) 
		auto texColglm = util::sLogSeverityData.at(util::LogSeverity::Info).second;
		auto texColimv = ImVec4(texColglm.x, texColglm.y, texColglm.z, 1.f);
		colors[ImGuiCol_Text] = texColimv;
		auto disabledTexColglm = util::sLogSeverityData.at(util::LogSeverity::Verbose).second;
		auto disabledTexColimv = ImVec4(disabledTexColglm.x, disabledTexColglm.y, disabledTexColglm.z, 1.f);
		colors[ImGuiCol_TextDisabled] = disabledTexColimv;
		colors[ImGuiCol_WindowBg] = HEXTOIM(0x070808);
		colors[ImGuiCol_Border] = HEXTOIM(0x434343);
		colors[ImGuiCol_FrameBg] = HEXTOIM(0x101B15);
		colors[ImGuiCol_FrameBgHovered] = HEXTOIM(0x393D3A);
		colors[ImGuiCol_FrameBgActive] = HEXTOIM(0x454f47);
		colors[ImGuiCol_TitleBg] = HEXTOIM(0x151917);
		colors[ImGuiCol_TitleBgActive] = HEXTOIM(0x1f2824);
		colors[ImGuiCol_DragDropTarget] = texColimv;
		colors[ImGuiCol_DockingPreview] = HEXTOIM(0x373f38);
		colors[ImGuiCol_TabHovered] = HEXTOIM(0x373f38);
		colors[ImGuiCol_TabActive] = HEXTOIM(0x394738);
		colors[ImGuiCol_Tab] = HEXTOIM(0x1F2622);
		colors[ImGuiCol_TabUnfocused] = HEXTOIM(0x1F2622);
		colors[ImGuiCol_TabUnfocusedActive] = HEXTOIM(0x19201C);
		colors[ImGuiCol_CheckMark] = texColimv;
		colors[ImGuiCol_SliderGrab] = disabledTexColimv;
		colors[ImGuiCol_SliderGrabActive] = texColimv;
		colors[ImGuiCol_Button] = HEXTOIM(0x1f2824);
		colors[ImGuiCol_ButtonHovered] = HEXTOIM(0x393D3A);
		colors[ImGuiCol_ButtonActive] = HEXTOIM(0x454f47);
		colors[ImGuiCol_Header] = HEXTOIM(0x171c19);
		colors[ImGuiCol_HeaderHovered] = HEXTOIM(0x393D3A);
		colors[ImGuiCol_HeaderActive] = HEXTOIM(0x454f47);
		colors[ImGuiCol_ResizeGrip] = HEXTOIM(0x373f38);
		colors[ImGuiCol_ResizeGripHovered] = disabledTexColimv;
		colors[ImGuiCol_ResizeGripActive] = texColimv;
		colors[ImGuiCol_SeparatorHovered] = disabledTexColimv;
		colors[ImGuiCol_SeparatorActive] = texColimv;
		// ImPlot::GetStyle().Colors[ImPlotCol_Line] = texColimv;

		style->FrameRounding = 0.0f;
		style->WindowBorderSize = 0.f;
		style->FrameBorderSize = 0.f;
		style->ChildBorderSize = 0.f;
		style->WindowPadding = { 4.f, 4.f };
		style->ItemSpacing = { 8.f, 4.f };
		style->GrabMinSize = 10.f;

		style->ChildRounding = 0.0f;
		style->PopupRounding = 1.0f;
		style->ScrollbarRounding = 0.0f;
		style->ScrollbarSize = 13.0f;
		style->TabBorderSize = 0.0f;
		style->TabRounding = 0.0f;
		style->WindowRounding = 0.0f;
		style->GrabRounding = 0.f;
		style->WindowMenuButtonPosition = ImGuiDir_None;
		style->ColorButtonPosition = ImGuiDir_Left;
		style->AntiAliasedFill = false;
		style->AntiAliasedFill = false;

		
		ImGui_ImplGlfw_InitForOpenGL(window, false);
		ImGui::GetIO().BackendFlags |= ImGuiBackendFlags_RendererHasViewports;
		ImGui::GetPlatformIO().Renderer_RenderWindow = weirdImGuiDrawData;

	}

	void ImGuiManager::update() {
		NEO_ASSERT(mIsEnabled, "ImGui is disabled");
		TRACY_ZONE();

		{
			TRACY_ZONEN("ImGui_ImplGlfw_NewFrame");
			ImGui_ImplGlfw_NewFrame();
		}
		{
			TRACY_ZONEN("ImGui::NewFrame");
			ImGui::NewFrame();
		}
		{
			TRACY_ZONEN("ImGuizmo::BeginFrame");
			ImGuizmo::BeginFrame();
		}

		if (isViewportHovered()) {
			TRACY_ZONEN("MouseMoveMessage");
			auto [mx, my] = ImGui::GetMousePos();
			mx -= mViewport.mOffset.x;
			my -= mViewport.mOffset.y;
			my = mViewport.mSize.y - my;
			if (mx >= 0 && mx <= mViewport.mSize.x && my >= 0 && my <= mViewport.mSize.y) {
				Messenger::sendMessage<Mouse::MouseMoveMessage>(static_cast<double>(mx), static_cast<double>(my));
			}
		}
	}

	void ImGuiManager::updateViewport() {
		NEO_ASSERT(mIsEnabled, "ImGui is disabled");
		TRACY_ZONE();

		// TODO -- this can all grab the necessary details directly using the Viewport's ImGuiID 
		mViewport.mIsFocused = ImGui::IsWindowFocused();
		mViewport.mIsHovered = ImGui::IsWindowHovered();

		auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
		auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
		auto viewportOffset = ImGui::GetWindowPos();
		glm::ivec2 offset = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
		glm::ivec2 size = { viewportMaxRegion.x - viewportMinRegion.x, viewportMaxRegion.y - viewportMinRegion.y };
		if (size.x != 0 && size.y != 0) {
			if (glm::uvec2(size) != mViewport.mSize || glm::uvec2(offset) != mViewport.mOffset) {
				mViewport.mOffset = glm::uvec2(offset);
				mViewport.mSize = glm::uvec2(size);
			}
			ImGuizmo::SetRect(
				static_cast<float>(offset.x),
				static_cast<float>(offset.y),
				static_cast<float>(size.x),
				static_cast<float>(size.y)
			);
		}
		ImGuizmo::SetImGuiContext(ImGui::GetCurrentContext());
	}

	void ImGuiManager::updateMouse(GLFWwindow* window, int button, int action, int mods) {
		NEO_ASSERT(mIsEnabled, "ImGui is disabled");

		ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
	}

	void ImGuiManager::updateKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods) {
		NEO_ASSERT(mIsEnabled, "ImGui is disabled");

		ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
	}

	void ImGuiManager::updateScroll(GLFWwindow* window, double dx, double dy) {
		NEO_ASSERT(mIsEnabled, "ImGui is disabled");

		ImGui_ImplGlfw_ScrollCallback(window, dx, dy);
	}

	void ImGuiManager::updateCharacter(GLFWwindow* window, unsigned int c) {
		NEO_ASSERT(mIsEnabled, "ImGui is disabled");

		ImGui_ImplGlfw_CharCallback(window, c);
	}

	void ImGuiManager::begin() {
		NEO_ASSERT(mIsEnabled, "ImGui is disabled");
		TRACY_ZONE();

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_HorizontalScrollbar;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		static bool open = true;
		ImGui::Begin("###DockSpace", &open, window_flags);
		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		NEO_ASSERT(io.ConfigFlags & ImGuiConfigFlags_DockingEnable, "");
		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_NoUndocking);
	}

	void ImGuiManager::end() {
		NEO_ASSERT(mIsEnabled, "ImGui is disabled");
		TRACY_ZONEN("ImGuiManager::end");

		ImGui::End();
	}

	void ImGuiManager::reload(ResourceManagers& resourceManagers) {
		TRACY_ZONE();

		uint8_t* pixels;
		int width, height;
		ImGuiIO io = ImGui::GetIO();
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
		ImGui::GetIO().Fonts->SetTexID(resourceManagers.mTextureManager.asyncLoad("ImGuiFont", TextureBuilder{
				TextureFormat {
					types::texture::Target::Texture2D,
					types::texture::InternalFormats::RGBA32_F,
				},
				glm::u16vec3(width, height, 0),
				pixels
			}).mHandle
		);

		for (int i = 0; i < 16; i++) {
			MeshLoadDetails loadDetails;
			loadDetails.mPrimtive = types::mesh::Primitive::Triangles;
			loadDetails.mVertexBuffers[types::mesh::VertexType::Position] = MeshLoadDetails::VertexBuffer{
				2,
				sizeof(ImVec2),
				types::ByteFormats::Float,
				false,
				0,
				0,
				0,
				nullptr
			};
			// HEHEHE STORE COLOR IN NORMAL HEHEHE
			loadDetails.mVertexBuffers[types::mesh::VertexType::Normal] = MeshLoadDetails::VertexBuffer{
				4,
				sizeof(ImU32),
				types::ByteFormats::UnsignedByte,
				true,
				0,
				0,
				0,
				nullptr
			};
			loadDetails.mVertexBuffers[types::mesh::VertexType::Texture0] = MeshLoadDetails::VertexBuffer{
				2,
				sizeof(ImVec2),
				types::ByteFormats::Float,
				false,
				0,
				0,
				0,
				nullptr
			};
			loadDetails.mElementBuffer = MeshLoadDetails::ElementBuffer{
				0,
				sizeof(ImDrawIdx) == 2 ? types::ByteFormats::UnsignedShort : types::ByteFormats::UnsignedInt,
				0,
				nullptr
			};

			std::string handle = "ImGuiMesh_" + std::to_string(i);
			mImGuiMeshes[i] = resourceManagers.mMeshManager.asyncLoad(HashedString(handle.c_str()), loadDetails);
		}
	}

	void ImGuiManager::resolveDrawData(ECS& ecs, ResourceManagers& resourceManagers) {
		TRACY_GPU();
		ImGui::Render();

		ImDrawData* drawData = ImGui::GetDrawData();
		NEO_ASSERT(drawData && drawData->Valid, "ImDrawData is invalid");
		if (drawData->CmdListsCount == 0) {
			return;
		}

		const ImVec2 clipOffset = drawData->DisplayPos;         // (0,0) unless using multi-viewports
		const ImVec2 clipScale = drawData->FramebufferScale; // (1,1) unless using retina display which are often (2,2)0

		uint32_t drawIndex = 0;
		for (int i = 0; i < drawData->CmdListsCount; i++) {
			const ImDrawList* cmdList = drawData->CmdLists[i];

			// Need to break this struct apart into individual buffers and upload them into individual VBOs...
			{
				std::vector<ImVec2> vertices;
				vertices.resize(cmdList->VtxBuffer.Size);
				std::vector<ImVec2> uvs;
				uvs.resize(cmdList->VtxBuffer.Size);
				std::vector<ImU32> colors;
				colors.resize(cmdList->VtxBuffer.Size);
				for (int j = 0; j < cmdList->VtxBuffer.Size; j++) {
					vertices[j] = cmdList->VtxBuffer[j].pos;
					uvs[j] = cmdList->VtxBuffer[j].uv;
					colors[j] = cmdList->VtxBuffer[j].col;
				}
				std::vector<ImDrawIdx> elements;
				elements.resize(cmdList->IdxBuffer.Size);
				memcpy(elements.data(), cmdList->IdxBuffer.Data, cmdList->IdxBuffer.Size * sizeof(ImDrawIdx));
				NEO_ASSERT(i < mImGuiMeshes.size(), "ImGui is requesting too many meshes :(");
				resourceManagers.mMeshManager.transact(mImGuiMeshes[i],
					[vertices = std::move(vertices),
					uvs = std::move(uvs),
					colors = std::move(colors),
					elements = std::move(elements)]
					(Mesh& mesh) {
						mesh.updateVertexBuffer(
							types::mesh::VertexType::Position,
							static_cast<uint32_t>(vertices.size()),
							static_cast<uint32_t>(vertices.size() * sizeof(ImVec2)),
							reinterpret_cast<const uint8_t*>(vertices.data())
						);
						mesh.updateVertexBuffer(
							types::mesh::VertexType::Texture0,
							static_cast<uint32_t>(uvs.size()),
							static_cast<uint32_t>(uvs.size() * sizeof(ImVec2)),
							reinterpret_cast<const uint8_t*>(uvs.data())
						);
						mesh.updateVertexBuffer(
							types::mesh::VertexType::Normal,
							static_cast<uint32_t>(colors.size()),
							static_cast<uint32_t>(colors.size() * sizeof(ImU32)),
							reinterpret_cast<const uint8_t*>(colors.data())
						);
						mesh.removeElementBuffer();
						mesh.addElementBuffer(
							static_cast<uint32_t>(elements.size()),
							sizeof(ImDrawIdx) == 2 ? types::ByteFormats::UnsignedShort : types::ByteFormats::UnsignedInt,
							static_cast<uint32_t>(elements.size() * sizeof(ImDrawIdx)),
							reinterpret_cast<const uint8_t*>(elements.data())
						);
						mesh.mPrimitiveType = types::mesh::Primitive::Triangles;
					});
			}

			for (int cmd_i = 0; cmd_i < cmdList->CmdBuffer.Size; cmd_i++) {
				const ImDrawCmd* cmd = &cmdList->CmdBuffer[cmd_i];
				if (cmd->UserCallback != nullptr) {
					if (cmd->UserCallback == ImDrawCallback_ResetRenderState) {
						NEO_LOG_W("Reset the render state wahoo");
					}
					else {
						NEO_LOG_W("Callback function!?");
						//cmd->UserCallback(cmdList, cmd);
					}
				}
				else {
					glm::vec2 clipMin = glm::vec2((cmd->ClipRect.x - clipOffset.x) * clipScale.x, (cmd->ClipRect.y - clipOffset.y) * clipScale.y);
					glm::vec2 clipMax = glm::vec2((cmd->ClipRect.z - clipOffset.x) * clipScale.x, (cmd->ClipRect.w - clipOffset.y) * clipScale.y);
					//if (clipMax.x <= clipMin.x || clipMax.y <= clipMin.y) {
						//continue;
					//}

					auto entity = ecs.createEntity();
					ecs.addComponent<ImGuiComponent>(entity);

					ImGuiDrawComponent* component = ecs.addComponent<ImGuiDrawComponent>(entity);
					component->mMeshHandle = mImGuiMeshes[i];
					component->mTextureHandle = TextureHandle(cmd->TextureId);
					component->mScissorRect = glm::vec4(
						clipMin.x,
						clipMax.y,
						clipMax.x - clipMin.x,
						clipMax.y - clipMin.y
					);
					component->mElementCount = static_cast<uint16_t>(cmd->ElemCount);
					component->mElementBufferOffset = static_cast<uint16_t>(cmd->IdxOffset * sizeof(ImDrawIdx));
					component->mDrawOrder = drawIndex++;
				}
			}
		}
	}

	void ImGuiManager::toggleImGui() {
		NEO_LOG_I("Toggle imgui");
		mIsEnabled = !mIsEnabled;
		reset();
	}

	void ImGuiManager::reset() {
		mViewport = {};
	}

	bool ImGuiManager::isViewportFocused() {
		NEO_ASSERT(mIsEnabled, "ImGui is disabled");

		return mViewport.mIsFocused;
	}

	bool ImGuiManager::isViewportHovered() {
		NEO_ASSERT(mIsEnabled, "ImGui is disabled");

		return mViewport.mIsHovered;
	}

	glm::uvec2 ImGuiManager::getViewportSize() {
		NEO_ASSERT(mIsEnabled, "ImGui is disabled");

		// Viewport isn't ready on the first frame..
		if (mViewport.mSize == glm::uvec2(0, 0)) {
			return glm::uvec2(1, 1);
		}
		return mViewport.mSize;
	}

	glm::uvec2 ImGuiManager::getViewportOffset() {
		NEO_ASSERT(mIsEnabled, "ImGui is disabled");

		return mViewport.mOffset;
	}

	void ImGuiManager::destroy() {
		ImGui_ImplGlfw_Shutdown();
		ImPlot::DestroyContext();
		ImGui::DestroyContext();
	}

	void ImGuiManager::log(const char* log, util::LogSeverity severity) {
		NEO_UNUSED(log, severity);
		mConsole.addLog(log, severity);
	}

	void ImGuiManager::imGuiEditor() {
		TRACY_ZONE();
		mConsole.imGuiEditor();
	}
}
