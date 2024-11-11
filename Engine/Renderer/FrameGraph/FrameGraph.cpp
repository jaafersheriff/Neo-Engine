#include "FrameGraph.hpp"

#include "Renderer/GLObjects/GLFrameGraphResolve.hpp"

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
		// 	output << "label=\"v" << mTasks[node2].mDebugName.value_or("empty") << "\",shape=\"box\"";
		// 	});
		// printf("%s\n", output.str().c_str());

		std::set<uint16_t> passSeq;
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
				if (passSeq.find(task.mPassIndex) == passSeq.end()) {
					passSeq.insert(task.mPassIndex);
					(*task.mPassBuilder)(mFrameData.getPass(task.mPassIndex), resourceManagers, ecs);
					for (auto e : graph.out_edges(vertex)) {
						nodesToVisit.push_back(e.second);
					}
				}
			}
			NEO_ASSERT(passSeq.size() == mTasks.size(), "Incorrect task exection count");
		}

		// Sequentially walk through passes, make GL calls
		for (auto& passID : passSeq) {
			TRACY_GPUN("Pass");
			Pass& pass = mFrameData.getPass(passID);
			for (uint16_t i = 0; i < pass.getCommandSize(); i++) {
				Command& c = pass.getCommand(i);
				GLFrameGraphResolve(mFrameData, pass, c, resourceManagers);
			}
		}
	}
}
