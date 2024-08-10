#include "ResourceManager/ResourceManagers.hpp"

#include "Util/RenderThread.hpp"
#include "Util/ServiceLocator.hpp"

#include <imgui.h>

namespace neo {

	void ResourceManagers::tick() {
		TRACY_ZONE();
		auto& renderThread = ServiceLocator<RenderThread>::ref();
		mMeshManager.tick(renderThread);
		mShaderManager.tick(renderThread);
		mTextureManager.tick(renderThread);
		mFramebufferManager.tick(mTextureManager, renderThread); // Do this after textures
	}

	void ResourceManagers::clear() {
		mMeshManager.clear();
		mShaderManager.clear();
		mTextureManager.clear();
		mFramebufferManager.clear(mTextureManager); // Do this after textures
	}

	void ResourceManagers::imguiEditor() {
		TRACY_ZONE();
		auto textureFunc = [&](const Texture& texture) {
			if (texture.mFormat.mTarget != types::texture::Target::Texture2D) {
				ImGui::Text("Non-2D texture");
				return;
			}
			float scale = 175.f / (texture.mWidth > texture.mHeight ? texture.mWidth : texture.mHeight);
#pragma warning(push)
#pragma warning(disable: 4312)
			ImGui::Image(reinterpret_cast<ImTextureID>(texture.mTextureID), ImVec2(scale * texture.mWidth, scale * texture.mHeight), ImVec2(0, 1), ImVec2(1, 0));
#pragma warning(pop)
			};

		ImGui::Begin("Resources");
		if (ImGui::TreeNodeEx(&mFramebufferManager, ImGuiTreeNodeFlags_DefaultOpen, "Framebuffers (%d)", mFramebufferManager.mCache.size())) {
			mFramebufferManager.imguiEditor(textureFunc, mTextureManager);
			ImGui::TreePop();
		}
		if (ImGui::TreeNodeEx(&mShaderManager, ImGuiTreeNodeFlags_DefaultOpen, "Shaders (%d)", mShaderManager.mCache.size())) {
			mShaderManager.imguiEditor();
			ImGui::TreePop();
		}
		if (ImGui::TreeNodeEx(&mTextureManager, ImGuiTreeNodeFlags_None, "Texture (%d)", mTextureManager.mCache.size())) {
			mTextureManager.imguiEditor(textureFunc);
			ImGui::TreePop();
		}
		if (ImGui::TreeNodeEx(&mMeshManager, ImGuiTreeNodeFlags_None, "Meshes (%d)", mMeshManager.mCache.size())) {
			mMeshManager.imguiEditor();
			ImGui::TreePop();
		}
		ImGui::End();
	}
}