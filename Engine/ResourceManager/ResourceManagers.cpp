#include "ResourceManager/ResourceManagers.hpp"

#include <ext/imgui_incl.hpp>

namespace neo {

	void ResourceManagers::_tick() {
		TRACY_GPU();
		mMeshManager.tick();
		mShaderBufferManager.tick();
		mShaderManager.tick();
		mTextureManager.tick();
		mFramebufferManager.tick(); // Do this after textures
	}

	void ResourceManagers::_clear() {
		mMeshManager.clear();
		mShaderBufferManager.clear();
		mShaderManager.clear();
		mTextureManager.clear();
		mFramebufferManager.clear(); // Do this after textures
	}

	void ResourceManagers::_imguiEditor() {
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
		if (ImGui::TreeNodeEx(&mShaderBufferManager, ImGuiTreeNodeFlags_None, "Shader Buffers (%d)", mShaderBufferManager.mCache.size())) {
			mShaderBufferManager.imguiEditor();
			ImGui::TreePop();
		}
		ImGui::End();
	}
}
