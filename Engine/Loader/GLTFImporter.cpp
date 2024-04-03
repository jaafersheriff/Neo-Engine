#include "GLTFImporter.hpp"

#include "Renderer/GLObjects/Mesh.hpp"
#include "Renderer/GLObjects/Texture.hpp"

#include "ResourceManager/ResourceManagers.hpp"

#pragma warning(push)
#pragma warning(disable: 4201)
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 4018)
#pragma warning(disable: 4100)
#pragma warning(disable: 4267)
#define TINYGLTF_USE_CPP14 
#include <tiny_gltf.h>
#include <stb_image.h>
#pragma warning(pop)

namespace {
	inline neo::types::mesh::Primitive _translateTinyGltfPrimitiveType(int mode) {
		switch (mode) {
		case TINYGLTF_MODE_POINTS:
			return neo::types::mesh::Primitive::Points;
			break;
		case TINYGLTF_MODE_LINE:
			return neo::types::mesh::Primitive::Line;
			break;
		case TINYGLTF_MODE_LINE_LOOP:
			return neo::types::mesh::Primitive::LineLoop;
			break;
		case TINYGLTF_MODE_LINE_STRIP:
			return neo::types::mesh::Primitive::LineStrip;
			break;
		case TINYGLTF_MODE_TRIANGLES:
			return neo::types::mesh::Primitive::Triangles;
			break;
		case TINYGLTF_MODE_TRIANGLE_STRIP:
			return neo::types::mesh::Primitive::TriangleStrip;
			break;
		case TINYGLTF_MODE_TRIANGLE_FAN:
			return neo::types::mesh::Primitive::TriangleFan;
			break;
		default:
			NEO_FAIL("Invalid primitive type: %d", mode);
			return neo::types::mesh::Primitive::TriangleStrip;
		}
	}

	inline neo::types::ByteFormats _translateTinyGltfComponentType(int tinyGltfComponentType) {
		switch (tinyGltfComponentType) {
		case TINYGLTF_COMPONENT_TYPE_BYTE:
			return neo::types::ByteFormats::Byte;
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
			return neo::types::ByteFormats::UnsignedByte;
		case TINYGLTF_COMPONENT_TYPE_SHORT:
			return neo::types::ByteFormats::Short;
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
			return neo::types::ByteFormats::UnsignedShort;
		case TINYGLTF_COMPONENT_TYPE_INT:
			return neo::types::ByteFormats::Int;
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
			return neo::types::ByteFormats::UnsignedInt;
		case TINYGLTF_COMPONENT_TYPE_DOUBLE:
			return neo::types::ByteFormats::Double;
		case TINYGLTF_COMPONENT_TYPE_FLOAT:
			return neo::types::ByteFormats::Float;
		default:
			NEO_FAIL("Invalid component type: %d", tinyGltfComponentType);
			return neo::types::ByteFormats::Float;
		}
	}

	inline neo::types::texture::InternalFormats _translateTinyGltfPixelType(int pixel_type, neo::types::texture::BaseFormats baseFormat) {
		switch (pixel_type) {
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
			if (baseFormat == neo::types::texture::BaseFormats::RGBA) {
				return neo::types::texture::InternalFormats::RGBA8_UNORM;
			}
			else if (baseFormat == neo::types::texture::BaseFormats::RGB) {
				return neo::types::texture::InternalFormats::RGB8_UNORM;
			}
			else if (baseFormat == neo::types::texture::BaseFormats::RG) {
				return neo::types::texture::InternalFormats::RG8_UNORM;
			}
			else if (baseFormat == neo::types::texture::BaseFormats::R) {
				return neo::types::texture::InternalFormats::R8_UNORM;
			}
			break;
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
			if (baseFormat == neo::types::texture::BaseFormats::RGBA) {
				return neo::types::texture::InternalFormats::RGBA16_UI;
			}
			else if (baseFormat == neo::types::texture::BaseFormats::RGB) {
				return neo::types::texture::InternalFormats::RGB16_UI;
			}
			else if (baseFormat == neo::types::texture::BaseFormats::RG) {
				return neo::types::texture::InternalFormats::RG16_UI;
			}
			else if (baseFormat == neo::types::texture::BaseFormats::R) {
				return neo::types::texture::InternalFormats::R16_UI;
			}
			break;
		case TINYGLTF_COMPONENT_TYPE_FLOAT:
			if (baseFormat == neo::types::texture::BaseFormats::RGBA) {
				return neo::types::texture::InternalFormats::RGBA32_F;
			}
			else if (baseFormat == neo::types::texture::BaseFormats::RGB) {
				return neo::types::texture::InternalFormats::RGB32_F;
			}
			else if (baseFormat == neo::types::texture::BaseFormats::RG) {
				return neo::types::texture::InternalFormats::RG32_F;
			}
			else if (baseFormat == neo::types::texture::BaseFormats::R) {
				return neo::types::texture::InternalFormats::R32_F;
			}
			break;
		default:
			NEO_FAIL("Unsupported pixel type %d", pixel_type);
		}

		NEO_FAIL("Invalid combo of internal/base format");
		return neo::types::texture::InternalFormats::RGBA8_UNORM;
	}

	inline neo::types::texture::BaseFormats _getGLBaseFormat(int components) {
		switch (components) {
		case  1:
			return neo::types::texture::BaseFormats::R;
		case  2:
			return neo::types::texture::BaseFormats::RG;
		case  3:
			return neo::types::texture::BaseFormats::RGB;
		case  4:
			return neo::types::texture::BaseFormats::RGBA;
		default:
			NEO_FAIL("Invalid number of components: %d", components);
			return neo::types::texture::BaseFormats::RGB;
		}
	}

	inline neo::types::ByteFormats _getGLType(int bits) {
		switch (bits) {
		case  8:
			return neo::types::ByteFormats::UnsignedByte;
		case  16:
			return neo::types::ByteFormats::UnsignedShort;
		case  32:
			return neo::types::ByteFormats::Float;
		default:
			NEO_FAIL("Unsupported bit depth %d", bits);
			return neo::types::ByteFormats::UnsignedByte;
		}
	}

	inline neo::types::texture::Filters _translateTinyGltfFilter(int filter) {
		switch (filter) {
		case TINYGLTF_TEXTURE_FILTER_NEAREST:
			return neo::types::texture::Filters::Nearest;
			break;
		case TINYGLTF_TEXTURE_FILTER_LINEAR:
			return neo::types::texture::Filters::Linear;
			break;
		case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
			return neo::types::texture::Filters::NearestMipmapNearest;
			break;
		case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
			return neo::types::texture::Filters::LinearMipmapNearest;
			break;
		case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
			return neo::types::texture::Filters::NearestMipmapLinear;
			break;
		case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
			return neo::types::texture::Filters::LinearMipmapLinear;
			break;
		default:
			NEO_FAIL("Unsupported texture filter");
			return neo::types::texture::Filters::Linear;
			break;
		}
	}

	inline neo::types::texture::Wraps _translateTinyGltfWrap(int wrap) {
		switch (wrap) {
		case TINYGLTF_TEXTURE_WRAP_REPEAT:
			return neo::types::texture::Wraps::Repeat;
			break;
		case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
			return neo::types::texture::Wraps::Clamp;
			break;
		case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
			return neo::types::texture::Wraps::Mirrored;
			break;
		default:
			NEO_FAIL("Unsupported texture wrap");
			return neo::types::texture::Wraps::Clamp;
			break;
		}
	}

	neo::TextureResourceManager::TextureHandle _loadTexture(neo::TextureResourceManager& textureManager, const tinygltf::Model& model, int index, int texCoord) {
		using namespace neo; 

		if (index == -1) {
			return 0;
		}
		if (texCoord > 0) {
			NEO_LOG_W("Texture wants to use a different texcoord? This probably won't work");
		}

		const auto& texture = model.textures[index];
		const auto& image = model.images[texture.source];

		if (!texture.name.empty()) {
			NEO_LOG_V("Processing texture %s", texture.name.c_str());
		}

		if (!image.uri.empty()) {
			TextureResourceManager::TextureHandle textureHandle(HashedString(image.uri.c_str()));
			if (textureManager.isValid(textureHandle) || textureManager.isQueued(textureHandle)) {
				NEO_LOG_V("Texture %s is already loaded -- skipping", image.uri.c_str());
				return textureHandle;
			}
		}

		TextureBuilder builder;
		builder.mFormat.mType = _getGLType(image.bits);
		builder.mFormat.mInternalFormat = _translateTinyGltfPixelType(image.pixel_type, _getGLBaseFormat(image.component));
		if (texture.sampler > -1) {
			const auto& sampler = model.samplers[texture.sampler];
			if (sampler.minFilter > -1) {
				builder.mFormat.mFilter.mMin = _translateTinyGltfFilter(sampler.minFilter);
			}
			if (sampler.magFilter > -1) {
				builder.mFormat.mFilter.mMag = _translateTinyGltfFilter(sampler.magFilter);
			}
			if (sampler.wrapS > -1) {
				builder.mFormat.mWrap.mS = _translateTinyGltfWrap(sampler.wrapS);
			}
			if (sampler.wrapT > -1) {
				builder.mFormat.mWrap.mT = builder.mFormat.mWrap.mR = _translateTinyGltfWrap(sampler.wrapS);
			}
		}

		builder.mDimensions.x = static_cast<uint16_t>(image.width);
		builder.mDimensions.y = static_cast<uint16_t>(image.height);
		builder.mData = image.image.data();
		// Heh?
		HashedString name = HashedString(reinterpret_cast<char*>(const_cast<uint8_t*>(builder.mData)));
		if (!image.uri.empty()) {
			NEO_LOG_I("Loaded texture %s", image.uri.c_str());
			name = HashedString(image.uri.c_str());
		}
		return textureManager.asyncLoad(name, builder);
	}

	void _processNode(const char* path, const int nodeID, neo::ResourceManagers& resourceManagers, const tinygltf::Model& model, const tinygltf::Node& node, glm::mat4 parentXform, neo::GLTFImporter::Scene& outScene) {
		using namespace neo;
		if (node.camera != -1 || node.light != -1) {
			return;
		}

		// Spatial
		SpatialComponent nodeSpatial;
		glm::mat4 localTransform(1.f);
		if (node.matrix.size() == 16) {
			localTransform = glm::mat4(glm::make_mat4(node.matrix.data()));
		}
		else {
			if (node.translation.size() == 3) {
				nodeSpatial.setPosition(glm::vec3(node.translation[0], node.translation[1], node.translation[2]));
			}
			if (node.scale.size() == 3) {
				nodeSpatial.setScale(glm::vec3(node.scale[0], node.scale[1], node.scale[2]));
			}
			if (node.rotation.size() == 4) {
				glm::quat q = glm::make_quat(node.rotation.data());
				nodeSpatial.setOrientation(glm::mat3_cast(q));
			}
			localTransform = nodeSpatial.getModelMatrix();
		}
		nodeSpatial.setModelMatrix(parentXform * localTransform);

		for (auto& child : node.children) {
			_processNode(path, child, resourceManagers, model, model.nodes[child], nodeSpatial.getModelMatrix(), outScene);
		}

		// Mesh
		if (node.mesh < 0) {
			return;
		}

		if (!node.name.empty()) {
			NEO_LOG_V("Processing node %s", node.name.c_str());
		}

		if (!model.meshes[node.mesh].name.empty()) {
			NEO_LOG_V("Processing mesh %s", model.meshes[node.mesh].name.c_str());
		}
		for (int i = 0; i < model.meshes[node.mesh].primitives.size(); i++) {
			const auto& gltfMesh = model.meshes[node.mesh].primitives[i];

			GLTFImporter::Node outNode;
			outNode.mName = node.name + std::to_string(i);
			outNode.mSpatial = nodeSpatial;

			MeshLoadDetails builder;
			builder.mPrimtive = _translateTinyGltfPrimitiveType(gltfMesh.mode);

			// Indices
			if (gltfMesh.indices > -1)
			{
				auto& accessor = model.accessors[gltfMesh.indices];
				NEO_ASSERT(!accessor.sparse.isSparse, "Sparse accessor unsupported");

				auto& bufferView = model.bufferViews[accessor.bufferView];
				NEO_ASSERT(bufferView.target == TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER, "Indices bufferview isn't an index buffer?");

				const auto& buffer = model.buffers[bufferView.buffer];

				builder.mElementBuffer = {
					static_cast<uint32_t>(accessor.count),
					_translateTinyGltfComponentType(accessor.componentType),
					static_cast<uint32_t>(bufferView.byteLength),
					// TODO - this offset math might be bad
					static_cast<const uint8_t*>(buffer.data.data()) + bufferView.byteOffset + accessor.byteOffset
				};
			}

			for (const auto& attribute : gltfMesh.attributes) {
				const auto& accessor = model.accessors[attribute.second];
				NEO_ASSERT(!accessor.sparse.isSparse, "Sparse accessor unsupported");

				types::mesh::VertexType vertexType = types::mesh::VertexType::Position;
				if (attribute.first == "POSITION") {
					if (accessor.maxValues.size() == 3) {
						outNode.mMax = glm::vec3(accessor.maxValues[0], accessor.maxValues[1], accessor.maxValues[2]);
					}
					if (accessor.minValues.size() == 3) {
						outNode.mMin = glm::vec3(accessor.minValues[0], accessor.minValues[1], accessor.minValues[2]);
					}
				}
				else if (attribute.first == "NORMAL") {
					vertexType = types::mesh::VertexType::Normal;
				}
				else if (attribute.first == "TEXCOORD_0") {
					vertexType = types::mesh::VertexType::Texture0;
				}
				else if (attribute.first == "TANGENT") {
					vertexType = types::mesh::VertexType::Tangent;
				}
				else if (attribute.first == "COLOR_0") {
					NEO_LOG_W("Unsupported attribute: COLOR_0");
					continue;
				}
				else {
					NEO_FAIL("TODO: unsupported attribute: %s", attribute.first.c_str());
					continue;
				}

				const auto& bufferView = model.bufferViews[accessor.bufferView];
				const auto& buffer = model.buffers[bufferView.buffer];

				// TODO : this is duplicating vertex data. Would be better to split glBufferData from glVertexAttribPointer
				builder.mVertexBuffers[vertexType] = {
					static_cast<uint32_t>(tinygltf::GetNumComponentsInType(accessor.type)),
					static_cast<uint32_t>(accessor.ByteStride(bufferView)),
					_translateTinyGltfComponentType(accessor.componentType),
					accessor.normalized,
					static_cast<uint32_t>(accessor.count),
					static_cast<uint32_t>(accessor.byteOffset),
					static_cast<uint32_t>(bufferView.byteLength),
					static_cast<const uint8_t*>(buffer.data.data()) + bufferView.byteOffset
				};
			}

			std::string name;
			if (!model.meshes[node.mesh].name.empty()) {
				name = model.meshes[node.mesh].name;
			}
			else {
				std::stringstream ss;
				ss << path << "_";
				ss << nodeID << "_";
				ss << i;
				name = ss.str();
			}
			NEO_LOG_I("Loaded mesh %s", name.c_str());
			outNode.mMeshHandle = resourceManagers.mMeshManager.asyncLoad(HashedString(name.c_str()), builder);

			if (gltfMesh.material > -1) {
				auto& material = model.materials[gltfMesh.material];

				if (!material.lods.empty()) {
					NEO_LOG_W("Material %s has LODs -- unsupported", material.name.c_str());
				}
				if (material.alphaCutoff != 0.5) {
					NEO_LOG_W("Material %s has nonstandard alpha cutoff: %0.2f -- unsupported", material.name.c_str(), material.alphaCutoff);
				}

				outNode.mMaterial.mDoubleSided = material.doubleSided;

				if (material.alphaMode == "OPAQUE") {
					outNode.mAlphaMode = GLTFImporter::Node::AlphaMode::Opaque;
				}
				else if (material.alphaMode == "MASK") {
					outNode.mAlphaMode = GLTFImporter::Node::AlphaMode::AlphaTest;
				}
				else if (material.alphaMode == "BLEND") {
					outNode.mAlphaMode = GLTFImporter::Node::AlphaMode::Transparent;
					NEO_LOG_W("Material %s is transparent -- unsupported", material.name.c_str());
				}

				outNode.mMaterial.mNormalMap = _loadTexture(resourceManagers.mTextureManager, model, material.normalTexture.index, material.normalTexture.texCoord);
				if (material.normalTexture.scale != 1.0) {
					NEO_LOG_W("Material %s normal map has non-uniform scale -- unsupported", material.name.c_str());
				}

				outNode.mMaterial.mOcclusionMap = _loadTexture(resourceManagers.mTextureManager, model, material.occlusionTexture.index, material.occlusionTexture.texCoord);
				if (material.occlusionTexture.strength != 1.0) {
					NEO_LOG_W("Material %s occlusion map has a non-uniform strength -- unsupported", material.name.c_str());
				}

				if (material.emissiveFactor.size() == 3) {
					outNode.mMaterial.mEmissiveFactor = glm::vec3(material.emissiveFactor[0], material.emissiveFactor[1], material.emissiveFactor[2]);
				}
				outNode.mMaterial.mEmissiveMap = _loadTexture(resourceManagers.mTextureManager, model, material.emissiveTexture.index, material.emissiveTexture.texCoord);

				outNode.mMaterial.mMetallic = static_cast<float>(material.pbrMetallicRoughness.metallicFactor);
				outNode.mMaterial.mRoughness = static_cast<float>(material.pbrMetallicRoughness.roughnessFactor);
				if (material.pbrMetallicRoughness.baseColorFactor.size() == 4) {
					outNode.mMaterial.mAlbedoColor = glm::vec4(
						material.pbrMetallicRoughness.baseColorFactor[0],
						material.pbrMetallicRoughness.baseColorFactor[1],
						material.pbrMetallicRoughness.baseColorFactor[2],
						material.pbrMetallicRoughness.baseColorFactor[3]
					);
				}
				outNode.mMaterial.mAlbedoMap = _loadTexture(resourceManagers.mTextureManager, model, material.pbrMetallicRoughness.baseColorTexture.index, material.pbrMetallicRoughness.baseColorTexture.texCoord);
				outNode.mMaterial.mMetallicRoughnessMap = _loadTexture(resourceManagers.mTextureManager, model, material.pbrMetallicRoughness.metallicRoughnessTexture.index, material.pbrMetallicRoughness.metallicRoughnessTexture.texCoord);
			}

			outScene.mMeshNodes.push_back(outNode);
		}

	}
}

namespace neo {
	namespace GLTFImporter {

		Scene loadScene(const std::string& path, glm::mat4 baseTransform, ResourceManagers& resourceManagers) {
			NEO_ASSERT(path.length() > 4 && path.substr(path.length() - 4, 4) == "gltf", "Unsupported file type");

			tinygltf::Model model;
			tinygltf::TinyGLTF loader;
			std::string err;
			std::string warn;

			bool ret = false;
			stbi_set_flip_vertically_on_load(false);
			ret = loader.LoadASCIIFromFile(&model, &err, &warn, path.c_str());

			if (!warn.empty()) {
				NEO_LOG_W("tingltf Warning: %s", warn.c_str());
			}
			if (!err.empty()) {
				NEO_LOG_E("tinygltf Error: %s", err.c_str());
			}

			NEO_ASSERT(ret, "tinygltf failed to parse %s", path.c_str());
			if (!ret) {
				return {};
			}

			// Translate tinygltf::Model to Loader::GltfScene
			NEO_LOG_V("Translating %s to neo", path.c_str());
			if (model.lights.size()) {
				NEO_LOG_W("%s contains lights - ignoring", path.c_str());
			}
			if (model.cameras.size()) {
				NEO_LOG_W("%s contains cameras - ignoring", path.c_str());
			}
			if (model.defaultScene < 0 || model.scenes.size() > 1) {
				NEO_LOG_W("%s has multiple scenes. Just using the default", path.c_str());
			}

			Scene outScene;
			for (const auto& nodeID : model.scenes[model.defaultScene].nodes) {
				const auto& node = model.nodes[nodeID];
				_processNode(path.c_str(), nodeID, resourceManagers, model, node, baseTransform, outScene);
			}

			NEO_LOG_I("Successfully parsed %s", path.c_str());
			return outScene;

		}
	}
}