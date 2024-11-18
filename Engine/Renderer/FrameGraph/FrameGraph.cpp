#include "FrameGraph.hpp"

#include "Renderer/GLObjects/GLFrameGraphResolve.hpp"
#include "Renderer/Renderer.hpp"

#include "Util/ServiceLocator.hpp"
#include <stdlib.h>

namespace neo {
	namespace {
		auto enttGraph(const entt::flow& flow) {
			TRACY_ZONE();
			return flow.graph();
		}
	}

	void FrameGraph::execute(const ResourceManagers& resourceManagers, const ECS& ecs) {
		TRACY_GPU();
		NEO_UNUSED(resourceManagers, ecs);

		auto graph = enttGraph(mBuilder);

		// std::ostringstream output{};
		// entt::dot(output, graph, [&](auto& output, auto vertex) {
		// 	auto node2 = mBuilder[vertex];
		// 	output << "label=\"" << mTasks[node2].mDebugName.value_or("empty") << "\",shape=\"box\"";
		// 	});
		// printf("%s\n", output.str().c_str());

		std::set<std::pair<entt::id_type, uint16_t>> passSeq;
		{

			TRACY_ZONEN("Traverse Graph");
			std::deque<entt::flow::basic_flow::graph_type::vertex_type> nodesToVisit;
			for (auto&& vertex : graph.vertices()) {
				if (auto in_edges = graph.in_edges(vertex); in_edges.begin() == in_edges.end()) {
					nodesToVisit.push_back(vertex);
				}
			}

			// Traverse graph, execute pass builder tasks (can be async)
			while (!nodesToVisit.empty()) {
				auto vertex = nodesToVisit.front();
				nodesToVisit.pop_front();
				auto& task = mTasks[mBuilder[vertex]];
				if (passSeq.find({ mBuilder[vertex], task.mPassIndex }) == passSeq.end()) {
					passSeq.insert({ mBuilder[vertex], task.mPassIndex });
					{
						TRACY_ZONEF("%s", task.mDebugName.value_or("Task").c_str());
						NEO_ASSERT(task.mPassBuilder.has_value(), "heh?");
						(*task.mPassBuilder)(mFrameData.getPass(task.mPassIndex), resourceManagers, ecs);
					}
					for (auto e : graph.out_edges(vertex)) {
						nodesToVisit.push_back(e.second);
					}
				}
			}
			NEO_ASSERT(passSeq.size() == mTasks.size(), "Incorrect task exection count");
			ServiceLocator<Renderer>::value().mStats.mNumPasses = static_cast<uint32_t>(passSeq.size());
		}

		// Sequentially walk through passes, make GL calls
		{
			for (auto& passID : passSeq) {
				TRACY_GPUF(mTasks[passID.first].mDebugName.value_or("Pass").c_str());
				Pass& pass = mFrameData.getPass(passID.second);
				// Sort results
				{
					TRACY_ZONEN("Sort draws");
					// Selection sort nooooooooooooooooooooooo
					for (uint16_t i = 0; i < pass.mCommandIndex; i++) {
						uint16_t min = i;
						const Command& a = pass.getCommand(i);
						if (static_cast<CommandType>(a >> (64 - 3) & 0b111) != CommandType::Draw) {
							continue;
						}
						ShaderDefinesFG::HashedShaderDefines definesHashA = ShaderDefinesFG::getDefinesHash(mFrameData.getDefines(static_cast<uint16_t>(a >> (64 - 3 - 10 - 10 - 10) & 0b1111111111)));

						for (uint16_t j = i + 1; j < pass.mCommandIndex; j++) {
							const Command& b = pass.getCommand(j);
							if (static_cast<CommandType>(b >> (64 - 3) & 0b111) != CommandType::Draw) {
								continue;
							}

							ShaderDefinesFG::HashedShaderDefines definesHashB = ShaderDefinesFG::getDefinesHash(mFrameData.getDefines(static_cast<uint16_t>(b >> (64 - 3 - 10 - 10 - 10) & 0b1111111111)));
							if (definesHashA < definesHashB) {
								min = j;
							}
						}
						if (min != i) {
							Command t = a;
							pass.mCommands[i] = pass.getCommand(min);
							pass.mCommands[min] = t;
						}
					}
				}
				
				for (uint16_t i = 0; i < pass.getCommandSize(); i++) {
					Command& c = pass.getCommand(i);
					GLFrameGraphResolve(mFrameData, pass, c, resourceManagers);
				}
			}
		}
	}
}
