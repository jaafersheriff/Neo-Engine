#include "Loader/pch.hpp"
#include "Library.hpp"

#include "Loader.hpp"

#include "Renderer/GLObjects/Framebuffer.hpp"

#include "Engine/ImGuiManager.hpp"

#include "ResourceManager/MeshResourceManager.hpp"

#include "Util/Profiler.hpp"

namespace neo {
	/* Library */
	std::unordered_map<std::string, Framebuffer*> Library::mFramebuffers;
	std::unordered_map<neo::PooledFramebufferDetails, std::vector<Library::PooledFramebuffer>> Library::mPooledFramebuffers;

	void Library::tick() {
		TRACY_ZONE();
		for (auto it = mPooledFramebuffers.begin(); it != mPooledFramebuffers.end();) {
			auto& tvList = it->second;
			for (auto tvIt = tvList.begin(); tvIt != tvList.end();) {
				if (tvIt->mFrameCount == 0) {
					tvIt->mFramebuffer->destroy();
					delete tvIt->mFramebuffer;
					tvIt = tvList.erase(tvIt);
				}
				else {
					if (tvIt->mUsedThisFrame) {
						tvIt->mUsedThisFrame = false;
					}
					else {
						tvIt->mFrameCount--;
					}
					tvIt++;
				}
			}

			if (tvList.empty()) {
				it = mPooledFramebuffers.erase(it);
			}
			if (it != mPooledFramebuffers.end()) {
				it++;
			}
		}
	}

	Framebuffer* Library::createFramebuffer(const std::string& name) {
		auto fb = new Framebuffer;
		fb->init();
		mFramebuffers.emplace(name, fb);
		NEO_LOG("Creating FBO %s", name.c_str());
		return fb;
	}

	Framebuffer* Library::getPooledFramebuffer(const PooledFramebufferDetails& details, std::optional<std::string> name) {
		TRACY_ZONE();
		PooledFramebuffer& pfb = _findPooledFramebuffer(details);
		pfb.mUsedThisFrame = true;
		pfb.mName = name;
		if (!pfb.mFramebuffer) {
			pfb.mFramebuffer = new Framebuffer;
			pfb.mFramebuffer->init();
			for (auto& format : details.mFormats) {
				types::texture::BaseFormats baseFormat = TextureFormat::deriveBaseFormat(format.mInternalFormat);
				if (baseFormat == types::texture::BaseFormats::Depth) {
					pfb.mFramebuffer->attachDepthTexture(details.mSize, format.mInternalFormat, format.mFilter, format.mWrap);
				}
				else if (baseFormat == types::texture::BaseFormats::DepthStencil) {
					pfb.mFramebuffer->attachStencilTexture(details.mSize, format.mFilter, format.mWrap);
				}
				else {
					pfb.mFramebuffer->attachColorTexture(details.mSize, format);
				}
			}
			if (pfb.mFramebuffer->mColorAttachments) {
				pfb.mFramebuffer->initDrawBuffers();
			}
			pfb.mFrameCount = 1;
		}

		if (pfb.mFrameCount < 5) {
			pfb.mFrameCount++;
		}

		pfb.mFramebuffer->bind();
		return pfb.mFramebuffer;
	}

	Library::PooledFramebuffer& Library::_findPooledFramebuffer(const PooledFramebufferDetails& details) {
		TRACY_ZONE();
		auto it = mPooledFramebuffers.find(details);

		// First time seeing this description
		if (it == mPooledFramebuffers.end()) {
			mPooledFramebuffers[details] = {};
			return mPooledFramebuffers[details].emplace_back(PooledFramebuffer{});
		}
		else {
			// There's already a list here, search it
			for (auto& existingFramebuffer : it->second) {
				// An unused resource exists
				if (!existingFramebuffer.mUsedThisFrame) {
					return existingFramebuffer;
				}
			}
			// No unused resources :( Make a new one
			return it->second.emplace_back(PooledFramebuffer{});
		}
	}

	void Library::clean() {
		NEO_LOG("Cleaning library...");
		for (auto& frameBuffer : mFramebuffers) {
			frameBuffer.second->destroy();
		}
		mFramebuffers.clear();
	}

	void Library::imGuiEditor() {
		TRACY_ZONE();
		auto textureFunc = [&](const Texture& texture) {
			if (texture.mFormat.mTarget != types::texture::Target::Texture2D) {
				return;
			}
			float scale = 175.f / (texture.mWidth > texture.mHeight ? texture.mWidth : texture.mHeight);
#pragma warning(push)
#pragma warning(disable: 4312)
				ImGui::Image(reinterpret_cast<ImTextureID>(texture.mTextureID), ImVec2(scale * texture.mWidth, scale * texture.mHeight), ImVec2(0, 1), ImVec2(1, 0));
#pragma warning(pop)
			};

		ImGui::Begin("Library");

		if (ImGui::TreeNodeEx("Framebuffers", ImGuiTreeNodeFlags_DefaultOpen)) {
			if (ImGui::BeginTable("##Framebuffers", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_PreciseWidths | ImGuiTableFlags_SizingStretchSame)) {
				ImGui::TableSetupColumn("Name/Size", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_PreferSortDescending);
				ImGui::TableSetupColumn("Attachments");
				ImGui::TableHeadersRow();
				for (auto& fbo : Library::mFramebuffers) {
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("%s", fbo.first.c_str());
					ImGui::Text("[%d, %d]", fbo.second->mTextures[0]->mWidth, fbo.second->mTextures[0]->mHeight);
					ImGui::TableSetColumnIndex(1);
					for (auto t = fbo.second->mTextures.begin(); t < fbo.second->mTextures.end(); t++) {
						textureFunc(**t);
						if (t != std::prev(fbo.second->mTextures.end())) {
							ImGui::SameLine();
						}
					}
				}
				for (auto& [details, tvList] : Library::mPooledFramebuffers) {
					for (auto& tv : tvList) {
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						if (tv.mName) {
							ImGui::Text("*%s", tv.mName.value().c_str());
						}
						ImGui::Text("[%d, %d]", tv.mFramebuffer->mTextures[0]->mWidth, tv.mFramebuffer->mTextures[0]->mHeight);
						ImGui::TableSetColumnIndex(1);
						for (auto t = tv.mFramebuffer->mTextures.begin(); t < tv.mFramebuffer->mTextures.end(); t++) {
							textureFunc(**t);
							if (t != std::prev(tv.mFramebuffer->mTextures.end())) {
								ImGui::SameLine();
							}
						}
					}
				}
				ImGui::EndTable();
			}
			ImGui::TreePop();
		}
		// if (ImGui::TreeNodeEx("Shaders", ImGuiTreeNodeFlags_DefaultOpen)) {
		// 	for (auto& shader : Library::mShaders) {
		// 		if (ImGui::TreeNode(shader.first.c_str())) {
		// 			shader.second->imguiEditor();
		// 			ImGui::TreePop();
		// 		}
		// 	}
		// 	ImGui::TreePop();
		// }
		// if (ImGui::TreeNodeEx("Meshes", ImGuiTreeNodeFlags_DefaultOpen)) {
		// 	for (auto& m : Library::mMeshes) {
		// 		ImGui::TextWrapped("%s", m.first.c_str());
		// 	}
		// 	ImGui::TreePop();
		// }
		// if (ImGui::TreeNodeEx("Textures", ImGuiTreeNodeFlags_DefaultOpen)) {
		// 	for (auto& t : Library::mTextures) {
		// 		if (ImGui::TreeNode(t.first.c_str())) {
		// 			ImGui::Text("[%d, %d]", t.second->mWidth, t.second->mHeight);
		// 			textureFunc(*t.second);
		// 			ImGui::TreePop();
		// 		}
		// 	}
		// 	ImGui::TreePop();
		// }
		ImGui::End();

	}
}
