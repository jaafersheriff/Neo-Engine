#include "ResourceManager/ResourceManagers.hpp"

#include <ext/imgui_incl.hpp>

namespace neo {

	void ResourceManagers::tick() {
		TRACY_GPU();
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
		auto textureFunc = [&](const TextureHandle& textureHandle) {
			if (!mTextureManager.isValid(textureHandle)) {
				ImGui::Text("Invalid texture");
			}
			else {
				Texture& texture = mTextureManager.resolve(textureHandle);
				if (texture.mFormat.mTarget != types::texture::Target::Texture2D) {
					ImGui::Text("Non-2D texture");
				}
				else {
					float scale = 175.f / (texture.mWidth > texture.mHeight ? texture.mWidth : texture.mHeight);
					ImGui::Image(textureHandle.mHandle, ImVec2(scale * texture.mWidth, scale * texture.mHeight), ImVec2(0, 1), ImVec2(1, 0));
				}
			}
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
