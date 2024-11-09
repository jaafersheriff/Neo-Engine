#include "FrameGraph.hpp"

namespace neo {
	void FrameGraph::execute(const ResourceManagers& resourceManagers, const ECS& ecs) {
		TRACY_GPU();
		NEO_UNUSED(resourceManagers, ecs);

		auto graph = mBuilder.graph();

		//std::ostringstream output{};
		//entt::dot(output, graph, [&](auto& output, auto vertex) {
		//	auto node2 = mBuilder[vertex];
		//	output << "label=\"v" << node2 << "\",shape=\"box\"";
		//	});
		//printf("%s\n", output.str().c_str());

		// TODO - sort
		std::deque<entt::flow::basic_flow::graph_type::vertex_type> nodesToVisit;
		for (auto&& vertex : graph.vertices()) {
			if (auto in_edges = graph.in_edges(vertex); in_edges.begin() == in_edges.end()) {
				nodesToVisit.push_back(vertex);
			}
		}

		std::vector<uint16_t> passSeq;
		while (!nodesToVisit.empty()) {
			auto vertex = nodesToVisit.front();
			nodesToVisit.pop_front();
			auto it = mTasks.find(mBuilder[vertex]);
			if (it != mTasks.end()) {
				auto& task = mTasks[mBuilder[vertex]];
				passSeq.push_back(task.mPassIndex);
				task.f(mPassQueue.getPass(task.mPassIndex), resourceManagers, ecs);
				for (auto e : graph.out_edges(vertex)) {
					nodesToVisit.push_back(e.second);
				}
				mTasks.erase(mBuilder[vertex]);
			}
		}

		if (mTasks.size()) {
			NEO_LOG_E("Dangling tasks!");
		}

		for (auto& passID : passSeq) {
			Pass& pass = mPassQueue.getPass(passID);
			NEO_LOG_V("Pass (%d)", pass.getCommandSize());
			for (uint16_t i = 0; i < pass.getCommandSize(); i++) {
				Command& c = pass.getCommand(i);
				CommandType type = static_cast<CommandType>(c >> (64 - 3) & 0b111);
				switch (type) {
				case CommandType::Clear:
					_clear(pass, c);
					break;
				case CommandType::Draw:
					_draw(pass, c, resourceManagers);
					break;
				case CommandType::StartPass:
					_startPass(pass, c, resourceManagers);
					break;
				default:
					NEO_FAIL("Invalid pass type");
					break;
				}
			}
		}
	}

	void FrameGraph::_startPass(Pass& pass, const Command& command, const ResourceManagers& resourceManagers) {
		NEO_LOG_V("\tStart Pass");
		uint8_t fbID = static_cast<uint8_t>(
			command >> (64 - 3 - 8) & 0xFF
			);
		uint8_t vpID = static_cast<uint8_t>(
			command >> (64 - 3 - 8 - 8) & 0xFF
			);

		const auto& fbHandle = mPassQueue.mFramebufferHandles[fbID];
		const auto& vp = mPassQueue.mViewports[vpID];

		if (resourceManagers.mFramebufferManager.isValid(fbHandle)) {
			resourceManagers.mFramebufferManager.resolve(fbHandle).bind();
			glViewport(vp.x, vp.y, vp.z, vp.w);
		}

		switch (pass.mPassState.mDepthTest) {
		case DepthTest::Enabled:
			glEnable(GL_DEPTH_TEST);
			break;
		case DepthTest::Disabled:
			glDisable(GL_DEPTH_TEST);
			break;
		default:
			break;
		}
	}

	void FrameGraph::_clear(Pass& pass, Command& command) {
		NEO_LOG_V("\tClear");
		const auto& clearParams = pass.mClearParams[static_cast<uint8_t>(
			command >> (64 - 3 - 3) & 0b111
		)];

		// temp framebuffer to make gl calls
		Framebuffer fb;
		fb.clear(clearParams.color, clearParams.clearFlags);
	}

	void FrameGraph::_draw(Pass& pass, Command& command, const ResourceManagers& resourceManagers) {
		NEO_LOG_V("\tDraw");
		const auto& shaderHandle = mPassQueue.mShaderHandles[pass.mShaderIndex];
		if (!resourceManagers.mShaderManager.isValid(shaderHandle)) {
			return;
		}

		const auto& mesh = pass.mMeshes[static_cast<uint16_t>(
			command >> (64 - 3 - 10) & 0b1111111111
		)];
		if (!resourceManagers.mMeshManager.isValid(mesh)) {
			return;
		}
		const auto& ubo = pass.mUBOs[static_cast<uint16_t>(
			command >> (64 - 3 - 10 - 10) & 0b1111111111
		)];
		const auto& drawDefines = pass.mShaderDefines[static_cast<uint16_t>(
			command >> (64 - 3 - 10 - 10 - 10) & 0b1111111111
		)];

		ShaderDefines defines;
		for (auto& [d, b] : pass.mPassDefines.mDefines) {
			if (b) {
				defines.set(d);
			}
		}
		for (auto& [d, b] : drawDefines.mDefines) {
			if (b) {
				defines.set(d);
			}
		}

		const auto& resolvedShader = resourceManagers.mShaderManager.resolveDefines(shaderHandle, defines);
		for (auto& [k, v] : pass.mPassUBO.mUniforms) {
			resolvedShader.bindUniform(k, v);
		}
		for (auto& [k, t] : pass.mPassUBO.mTextures) {
			if (resourceManagers.mTextureManager.isValid(t)) {
				resolvedShader.bindTexture(k, resourceManagers.mTextureManager.resolve(t));
			}
		}
		for (auto& [k, v] : ubo.mUniforms) {
			resolvedShader.bindUniform(k, v);
		}
		for (auto& [k, t] : ubo.mTextures) {
			if (resourceManagers.mTextureManager.isValid(t)) {
				resolvedShader.bindTexture(k, resourceManagers.mTextureManager.resolve(t));
			}
		}

		resourceManagers.mMeshManager.resolve(mesh).draw();
	}
}