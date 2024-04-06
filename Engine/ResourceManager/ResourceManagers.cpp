#include "ResourceManager/ResourceManagers.hpp"

#include <imgui.h>

namespace neo {

	void ResourceManagers::tick() {
		TRACY_ZONE();
		mMeshManager.tick();
		mShaderManager.tick();
		mTextureManager.tick();
		mFramebufferManager.tick(mTextureManager); // Do this after textures
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
		if (ImGui::TreeNodeEx("Framebuffers", ImGuiTreeNodeFlags_DefaultOpen)) {
			mFramebufferManager.imguiEditor(textureFunc, mTextureManager);
			ImGui::TreePop();
		}
		if (ImGui::TreeNodeEx("Shaders", ImGuiTreeNodeFlags_DefaultOpen)) {
			mShaderManager.imguiEditor();
			ImGui::TreePop();
		}
		if (ImGui::TreeNodeEx("Textures", ImGuiTreeNodeFlags_DefaultOpen)) {
			mTextureManager.imguiEditor(textureFunc);
			ImGui::TreePop();
		}
		if (ImGui::TreeNodeEx("Meshes", ImGuiTreeNodeFlags_DefaultOpen)) {
			mMeshManager.imguiEditor();
			ImGui::TreePop();
		}
		ImGui::End();
	}
}