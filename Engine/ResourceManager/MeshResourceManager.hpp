#pragma once

#include "Renderer/GLObjects/Mesh.hpp"

#include "Util/Util.hpp"

#include <entt/resource/cache.hpp>

#include <memory>

namespace neo {

	struct MeshLoader final: entt::resource_loader<MeshLoader, Mesh> {
		std::shared_ptr<Mesh> load(Mesh mesh) const {
			return std::make_shared<Mesh>(mesh);
		}
	};

	using MeshCache = entt::resource_cache<Mesh>;
	using MeshHandle = entt::id_type;
	class MeshManager {
	public:
		MeshCache meshCache;

		Mesh& get(HashedString id) {
			if (meshCache.contains(id)) {
				return meshCache.handle(id).get();
			}
			NEO_LOG_E("Invalid mesh %s requested", id.data());
			return meshCache.handle(HashedString("cube")).get();
		}

		const Mesh& get(HashedString id) const {
			if (meshCache.contains(id)) {
				return meshCache.handle(id).get();
			}
			NEO_LOG_E("Invalid mesh %s requested", id.data());
			return meshCache.handle(HashedString("cube")).get();
		}

		Mesh& get(MeshHandle id) {
			if (meshCache.contains(id)) {
				return meshCache.handle(id).get();
			}
			NEO_LOG_E("Invalid mesh requested");
			return meshCache.handle(HashedString("cube")).get();
		}

		const Mesh& get(MeshHandle id) const {
			if (meshCache.contains(id)) {
				return meshCache.handle(id).get();
			}
			NEO_LOG_E("Invalid mesh requested");
			return meshCache.handle(HashedString("cube")).get();
		}

		MeshHandle load(HashedString id, Mesh mesh) {
			meshCache.load<MeshLoader>(id, mesh);
			return id;
		}

		void clear() {
			// meshCache.each([](Mesh& mesh) {
			// 	mesh.mMesh.destroy();
			// });
			meshCache.clear();
		}
		void imguiEditor() {
			meshCache.each([](const entt::id_type id) {
				NEO_UNUSED(id);
				// ImGuiText
			});
		}
	private:
	};
}