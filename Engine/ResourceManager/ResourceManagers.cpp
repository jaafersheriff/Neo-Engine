#include "ResourceManager/ResourceManagers.hpp"

#include <ext/imgui_incl.hpp>

namespace neo {

	void ResourceManagers::_tick() {
		TRACY_GPU();
		mMeshManager.tick();
		mShaderBufferManager.tick();
		mShaderManager.tick();
		mTextureManager.tick();
		mFramebufferManager.tick(mTextureManager); // Do this after textures
	}

	void ResourceManagers::_clear() {
		mMeshManager.clear();
		mShaderBufferManager.clear();
		mShaderManager.clear();
		mTextureManager.clear();
		mFramebufferManager.clear(mTextureManager); // Do this aft
	}

	void ResourceManagers::_imguiEditor() {
		auto textureFunc = [&](const TextureHandle& textureHandle, int arrayLayer = 0, int mip = 0) {
			if (!mTextureManager.isValid(textureHandle)) {
				ImGui::Text("Invalid texture");
			}
			else {
				Texture& texture = mTextureManager.resolve(textureHandle);
				const float scale = 175.f / (texture.mWidth > texture.mHeight ? texture.mWidth : texture.mHeight);
				ImGui::TextureDescriptor descriptor;
				descriptor.mTextureHandle = textureHandle.mHandle;
				descriptor.mArrayLayer = arrayLayer;
				descriptor.mMipLevel = mip;
				ImGui::PushID(textureHandle.mHandle + static_cast<int>(util::genRandom()));

				int maxDepth = texture.mDepth;
				switch (texture.mFormat.mTarget) {
				case types::texture::Target::Texture2D:
				case types::texture::Target::Texture2DArray:
				case types::texture::Target::TextureCube:
					maxDepth = 6;
				case types::texture::Target::Texture3D:
					if (texture.mDepth > 1 || texture.mFormat.mTarget == types::texture::Target::TextureCube) {
						ImGui::SliderInt("Array Layer", &arrayLayer, 0, maxDepth - 1);
						descriptor.mArrayLayer = arrayLayer;
					}
					if (texture.mFormat.mMipCount > 1) {
						ImGui::SliderInt("Mip Level", &mip, 0, texture.mFormat.mMipCount - 1);
						descriptor.mMipLevel = mip;;
					}
					ImGui::Image(descriptor, ImVec2(scale * texture.mWidth, scale * texture.mHeight), ImVec2(0, 1), ImVec2(1, 0));
					break;
				default:
					ImGui::Text("Unsupported texture");
					break;
				}
				ImGui::PopID();
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
