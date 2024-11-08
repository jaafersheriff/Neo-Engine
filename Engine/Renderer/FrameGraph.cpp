#include "FrameGraph.hpp"

namespace neo {
	void FrameGraph::execute(const ResourceManagers& resourceManagers, const ECS& ecs) {
		NEO_UNUSED(resourceManagers, ecs);

		auto graph = mBuilder.graph();

		std::ostringstream output{};
		entt::dot(output, graph, [&](auto& output, auto vertex) {
			auto node2 = mBuilder[vertex];
			output << "label=\"v" << node2 << "\",shape=\"box\"";
			});
		printf("%s\n", output.str().c_str());

		// TODO - sort
		std::deque<entt::flow::basic_flow::graph_type::vertex_type> nodesToVisit;
		for (auto&& vertex : graph.vertices()) {
			if (auto in_edges = graph.in_edges(vertex); in_edges.begin() == in_edges.end()) {
				nodesToVisit.push_back(vertex);
			}
		}

		while (!nodesToVisit.empty()) {
			auto vertex = nodesToVisit.front();
			nodesToVisit.pop_front();
			auto it = mTasks.find(mBuilder[vertex]);
			if (it != mTasks.end()) {
				mTasks[mBuilder[vertex]](resourceManagers, ecs);
				for (auto e : graph.out_edges(vertex)) {
					nodesToVisit.push_back(e.second);
				}
			}
		}

		if (mTasks.size()) {
			NEO_LOG_E("Dangling tasks!");
		}
	}
}