#pragma once

#include "ECS/ECS.hpp"
#include "ResourceManager/ResourceManagers.hpp"
#include "GLObjects/ResolvedShaderInstance.hpp"

#include <ext/entt_incl.hpp>

namespace neo {
	enum class DepthFunc {
		Less
	};
	enum class CullOrder {
		Front,
		Back
	};
	enum class BlendEquation {
		Add
	};
	enum class BlendFactor {
		One,
		Alpha,
		OneMinusAlpha
	};
	struct PassState {
		PassState() {
			TRACY_ZONE();
		}
		bool mDepthTest = true;
		DepthFunc mDepthFunc = DepthFunc::Less;
		bool mDepthMask = true;

		bool mCullFace = true;
		CullOrder mCullOrder = CullOrder::Back;

		bool mBlending = false;
		BlendEquation mBlendEquation = BlendEquation::Add;
		BlendFactor mBlendSrcRGB = BlendFactor::Alpha;
		BlendFactor mBlendDstRGB = BlendFactor::OneMinusAlpha;
		BlendFactor mBlendSrcAlpha = BlendFactor::Alpha;
		BlendFactor mBlendDstAlpha = BlendFactor::OneMinusAlpha;

		bool mStencilTest = false;
		bool mScissorTest = false;
	};

	using Viewport = glm::uvec4;

	struct UBO {
		UBO() {
			mUniformIndex = 0;
			mTextureIndex = 0;
		}
		void bindUniform(const char* name, const ResolvedShaderInstance::UniformVariant& variant) {
			mUniforms[mUniformIndex++] = { HashedString(name).value(), variant };
		}

		void bindTexture(const char* name, TextureHandle handle) {
			mTextures[mTextureIndex] = { HashedString(name).value(), handle };
		}

		// TODO - store as enum type pair
		struct UniformPair {
			entt::id_type bindhash; 
			ResolvedShaderInstance::UniformVariant val;
		};
		UniformPair mUniforms[128];
		uint16_t mUniformIndex = 0;

		struct TexturePair {
			entt::id_type bindhash; 
			TextureHandle val;
		};
		TexturePair mTextures[16];
		uint16_t mTextureIndex = 0;
	};

	struct ShaderDefineFG {
		ShaderDefineFG(const char* c = nullptr) {
			NEO_FAIL("NO");
			if (c) {
				mVal = reinterpret_cast<char*>(malloc(64));
				if (mVal) {
					strcpy(mVal, c);
				}
				else {
					mVal = 0;
				}
			}
		}
		ShaderDefineFG(const ShaderDefineFG& other) {
			NEO_FAIL("NO");
			if (other.mVal) {
				if (mVal) {
					free(mVal);
					mVal = 0;
				}
				mVal = reinterpret_cast<char*>(malloc(64));
				if (mVal) {
					strcpy(mVal, other.mVal);
				}
				else {
					mVal = 0;
				}
			}

		}
		~ShaderDefineFG() {
			if (mVal) {
				free(mVal);
			}
		}
		char* mVal = nullptr;

		friend bool operator<(const ShaderDefineFG& l, const ShaderDefineFG& r) {
			return HashedString(l.mVal).value() < HashedString(r.mVal).value();
		}
	};
	struct ShaderDefinesFG {
		ShaderDefinesFG() {
			reset();
		}
		ShaderDefinesFG(const ShaderDefinesFG& other) {
			reset();
			for (int i = 0; i < other.mDefinesIndex; i++) {
				set(ShaderDefine(other.mDefines[i].mVal));
			}
		}
		void set(const ShaderDefinesFG&& other) {
			reset();
			for (int i = 0; i < other.mDefinesIndex; i++) {
				set(ShaderDefine(other.mDefines[i].mVal));
			}
		}
		void set(const ShaderDefine& define) {
			mDefines[mDefinesIndex++] = ShaderDefineFG(define.mVal.c_str());
		}
		void reset() {
			mDefinesIndex = 0;
			for (int i = 0; i < 32; i++) {
				mDefines[i].~ShaderDefineFG();
			}
		}

		ShaderDefineFG mDefines[32];
		uint16_t mDefinesIndex = 0;
	};


	using Command = uint64_t;
	enum class CommandType : uint8_t {
		StartPass = 0,
		Clear = 1,
		Draw = 2
	};

	struct Pass {
		// 0-3: StartPass Key
		// 4-12: FBO ID
		// 13-21: Viewport ID
		void startCommand() {
			Command& command = mCommands[mCommandIndex++];
			command = 0
				| static_cast<uint64_t>(CommandType::StartPass) << (64 - 3)
				| static_cast<uint64_t>(mFramebufferIndex) << (64 - 3 - 8)
				| static_cast<uint64_t>(mViewportIndex) << (64 - 3 - 8 - 8)
			;
		}

		void drawCommand(const MeshHandle& mesh, const UBO& ubo, const ShaderDefinesFG&& drawDefines, uint16_t elements = 0, uint16_t bufferOffset = 0) {
			uint64_t drawIndex = mDrawIndex;
			mDraws[mDrawIndex++] = { mesh, elements, bufferOffset };
			uint64_t uboIndex = mUBOIndex;
			mUBOs[mUBOIndex++] = ubo;
			uint64_t definesIndex = mShaderDefinesIndex;
			mShaderDefines[mShaderDefinesIndex++].set(std::move(drawDefines));

			Command& command = mCommands[mCommandIndex++];
			command = 0
				| static_cast<uint64_t>(CommandType::Draw) << (64 - 3)
				| drawIndex << (64 - 3 - 10)
				| uboIndex << (64 - 3 - 10 - 10)
				| definesIndex << (64 - 3 - 10 - 10 - 10)
			;
		}

		ShaderDefinesFG& createShaderDefines() {
			return mShaderDefines[mShaderDefinesIndex++];
		}
		void setDefine(const ShaderDefine& define) {
			mPassDefines.set(define);
		}

		UBO& createUBO() {
			return mUBOs[mUBOIndex++];
		}
		void bindUniform(const char* name, const ResolvedShaderInstance::UniformVariant& variant) {
			mPassUBO.bindUniform(name, variant);
		}
		void bindTexture(const char* name, TextureHandle handle) {
			mPassUBO.bindTexture(name, handle);
		}

		struct ClearParams {
			glm::vec4 color;
			types::framebuffer::AttachmentBits clearFlags = 0;
		};
		void clearCommand(glm::vec4 color, types::framebuffer::AttachmentBits clearFlags) {
			mClearParams[mClearParamsIndex] = ClearParams{ color, clearFlags };

			Command& command = mCommands[mCommandIndex++];
			command = 0
				| static_cast<uint64_t>(CommandType::Clear) << (64 - 3)
				| static_cast<uint64_t>(mClearParamsIndex) << (64 - 3 - 3)
			;
			mClearParamsIndex++;
		}

		uint16_t getCommandSize() const { return mCommandIndex; }
		Command& getCommand(uint16_t index) { return mCommands[index]; }
	//private:
		uint8_t mFramebufferIndex = 0;
		uint8_t mViewportIndex = 0;
		uint8_t mScissorIndex = 0;
		uint8_t mShaderIndex = 0;
		UBO mPassUBO;
		ShaderDefinesFG mPassDefines;
		PassState mPassState;

		Command mCommands[1024];
		uint16_t mCommandIndex = 0;

		ClearParams mClearParams[8];
		uint8_t mClearParamsIndex = 0; // 3 bits

		ShaderDefinesFG mShaderDefines[1024];
		uint16_t mShaderDefinesIndex = 0; // 10 bits

		UBO mUBOs[1024];
		uint16_t mUBOIndex = 0; // 10 bits

		struct Draw {
			MeshHandle mMeshHandle;
			uint16_t mElementCount = 0;
			uint16_t mElementBufferOffset = 0;
		};
		Draw mDraws[1024];
		uint16_t mDrawIndex = 0; // 10 bits
	};


	struct PassQueue {
		~PassQueue() {
			TRACY_ZONE();
			for (auto pass : mCommandQueues) {
				free(reinterpret_cast<void*>(pass));
			}
		}

		uint16_t addPass(FramebufferHandle handle, Viewport vp, Viewport scissor, PassState& state, ShaderHandle shaderHandle = {}) {
			TRACY_ZONE();
			mFramebufferHandles[mFramebufferHandleIndex] = handle;
			mShaderHandles[mShaderHandleIndex] = shaderHandle;
			auto vpId = mViewportIndex;
			mViewports[mViewportIndex++] = vp;
			auto scId = vpId;
			if (vp != scissor) {
				scId = mViewportIndex;
				mViewports[mViewportIndex++] = scissor;
			}
			{
				TRACY_ZONEN("Construct pass");
				void* pm = malloc(sizeof(Pass));
				if (pm) {
					Pass* p = reinterpret_cast<Pass*>(pm);
					p->mFramebufferIndex = mFramebufferHandleIndex++;
					p->mViewportIndex = vpId;
					p->mScissorIndex = scId;
					p->mShaderIndex = mShaderHandleIndex++;
					p->mPassState = state;
					p->mPassUBO = UBO();
					p->mPassDefines.mDefinesIndex = 0;
					p->mCommandIndex = 0;
					p->mClearParamsIndex = 0;
					p->mShaderDefinesIndex = 0;
					p->mUBOIndex = 0;
					p->mDrawIndex = 0;
					mCommandQueues.push_back(p);
				}
				else {
					NEO_FAIL("Err");
				}
			}
			return static_cast<uint16_t>(mCommandQueues.size() - 1);
		}

		Pass& getPass(uint16_t index) {
			return *mCommandQueues[index];
		}

		FramebufferHandle mFramebufferHandles[256];
		uint8_t mFramebufferHandleIndex = 0;

		Viewport mViewports[256];
		uint8_t mViewportIndex = 0;

		ShaderHandle mShaderHandles[256];
		uint8_t mShaderHandleIndex = 0;

	private:
		std::vector<Pass*> mCommandQueues;
	};

	class FrameGraph {
	public:
		struct Task {
			using Functor = std::function<void(Pass&, const ResourceManagers&, const ECS&)>;
			Task()
				: mPassIndex(UINT16_MAX)
				, f([](Pass&, const ResourceManagers&, const ECS&) {})
			{}
			Task(uint16_t passIndex, Functor _f) :
				mPassIndex(passIndex),
				f(_f)
			{}

			uint16_t mPassIndex;
			Functor f;
			std::optional<std::string> mDebugName;
		};

		void execute(const ResourceManagers& resourceManagers, const ECS& ecs);

		template<typename... Deps>
		Task& pass(FramebufferHandle target, Viewport viewport, Viewport scissor, PassState state, ShaderHandle shader, Task::Functor t, Deps... deps) {
			TRACY_ZONE();
			uint16_t passIndex = mPassQueue.addPass(target, viewport, scissor, state, shader);
			return _task(std::move(Task(passIndex, [_t = std::move(t)](Pass& pass, const ResourceManagers& resourceManager, const ECS& ecs) {
				pass.startCommand();
				_t(pass, resourceManager, ecs);
			})), target, std::forward<Deps>(deps)...);
		}

		Task& clear(FramebufferHandle handle, glm::vec4 color, types::framebuffer::AttachmentBits clearFlags) {
			return pass(handle, {}, {}, {}, {}, [=](Pass& pass, const ResourceManagers&, const ECS&) {
				pass.clearCommand(color, clearFlags);
			});
		}

	private:
		// TODO - these should go elsewhere
		void _startPass(const Pass& pass, const Command& command, const ResourceManagers& resourceManagers);
		void _clear(const Pass& pass, Command& command);
		void _draw(const Pass& pass, Command& command, const ResourceManagers& resourceManagers);

		template<typename... Deps>
		Task& _task(Task&& t, Deps... deps) {
			TRACY_ZONE();
			entt::id_type taskHandle = mTasks.size();
			mBuilder.bind(taskHandle);
			_assignDeps(std::forward<Deps>(deps)...);

			mTasks.emplace_back(std::move(t));
			return mTasks.back();
		}

		template<typename Dep, typename... Deps>
		void _assignDeps(Dep dep, Deps... deps) {
			if constexpr (std::is_same_v<Dep, FramebufferHandle>) {
				mBuilder.rw(dep.mHandle);
			}
			else {
				static_assert(false, "dead");
			}
			if constexpr (sizeof...(Deps) > 0) {
				_assignDeps(deps...);
			}
		}

		std::vector<Task> mTasks;
		entt::flow mBuilder;
			
		PassQueue mPassQueue;
	};
}
