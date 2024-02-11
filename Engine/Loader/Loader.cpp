#include "Loader/pch.hpp"
#include "Loader.hpp"

#include "Library.hpp"

#include "Renderer/GLObjects/Mesh.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"

#include "Util/Profiler.hpp"

// DEPRECATED :O
#pragma warning(push)
#pragma warning(disable: 4706)
#define TINYOBJLOADER_IMPLEMENTATION
#include "ext/tiny_obj_loader.h"
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 4244)
#define STB_IMAGE_IMPLEMENTATION
#include "ext/stb_image.h"
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 4018)
#pragma warning(disable: 4100)
#pragma warning(disable: 4267)
#define TINYGLTF_NO_EXTERNAL_IMAGE 
#define TINYGLTF_USE_CPP14 
#define TINYGLTF_NO_STB_IMAGE 
#define TINYGLTF_NO_STB_IMAGE_WRITE 
#define TINYGLTF_IMPLEMENTATION
#include <tiny_gltf.h>
#pragma warning(pop)

#pragma optimize("", off)

namespace {
	inline GLenum _getGLFormat(int tinyGltfComponentType) {
		switch (tinyGltfComponentType) {
		case TINYGLTF_COMPONENT_TYPE_BYTE:
			return GL_BYTE;
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
			return GL_UNSIGNED_BYTE;
		case TINYGLTF_COMPONENT_TYPE_SHORT:
			return GL_SHORT;
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
			return GL_UNSIGNED_SHORT;
		case TINYGLTF_COMPONENT_TYPE_INT:
			return GL_INT;
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
			return GL_UNSIGNED_INT;
		case TINYGLTF_COMPONENT_TYPE_DOUBLE:
			return GL_DOUBLE;
		case TINYGLTF_COMPONENT_TYPE_FLOAT:
		default:
			return GL_FLOAT;
		}
	}
}

namespace neo {

	std::string Loader::APP_RES_DIR = "";
	std::string Loader::APP_SHADER_DIR = "";
	std::string Loader::ENGINE_RES_DIR = "../Engine/res/";
	std::string Loader::ENGINE_SHADER_DIR = "../Engine/shaders/";

	void Loader::init(const std::string &res, const std::string& shaderDir) {
		APP_RES_DIR = res;
		APP_SHADER_DIR = shaderDir;
	}

	const char* Loader::loadFileString(const std::string& fileName) {
		auto load = [](const char* fullPath, const char** ret) {
			if (util::fileExists(fullPath)) {
				// Each of these util::textFileReads does a malloc..
				*ret = util::textFileRead(fullPath);
				return true;
			}
			return false;
		};

		const char* ret;
		// TODO - this should be extended to other load funcs..
		if (load((APP_RES_DIR + fileName).c_str(), &ret)) {
			return ret;
		}
		if (load((APP_SHADER_DIR + fileName).c_str(), &ret)) {
			return ret;
		}
		if (load((ENGINE_RES_DIR + fileName).c_str(), &ret)) {
			return ret;
		}
		if (load((ENGINE_SHADER_DIR + fileName).c_str(), &ret)) {
			return ret;
		}
		NEO_LOG_E("Unable to find string file %s", fileName.c_str());
		return nullptr;
	}

	MeshData Loader::loadMesh_DEPRECATED(const std::string &fileName, bool doResize) {
		TRACY_ZONE();

		/* Create mesh */
		MeshData meshData;
		Mesh* mesh = new Mesh;
		meshData.mMesh = mesh;

		/* If mesh was not found in map, read it in */
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> objMaterials;
		std::string errString;
		std::string _fileName = APP_RES_DIR + fileName;
		if (!util::fileExists(_fileName.c_str())) {
			_fileName = ENGINE_RES_DIR + fileName;
		}
		NEO_ASSERT(util::fileExists(_fileName.c_str()), "Unable to find file: %s after checking:\n\t%s\n\t%s\n", fileName.c_str(), APP_RES_DIR.c_str(), ENGINE_RES_DIR.c_str());
		// TODO : use assimp or another optimized asset loader
		bool rc = tinyobj::LoadObj(shapes, objMaterials, errString, _fileName.c_str());
		NEO_ASSERT(rc, errString.c_str());

		/* Create empty mesh buffers */
		std::vector<float> vertices;
		std::vector<float> normals;
		std::vector<float> texCoords;
		std::vector<uint32_t> indices;

		int vertCount = 0;
		/* For every shape in the loaded file */
		for (uint32_t i = 0; i < shapes.size(); i++) {
			/* Concatenate the shape's vertices, normals, and textures to the mesh */
			vertices.insert(vertices.end(), shapes[i].mesh.positions.begin(), shapes[i].mesh.positions.end());
			normals.insert(normals.end(), shapes[i].mesh.normals.begin(), shapes[i].mesh.normals.end());
			texCoords.insert(texCoords.end(), shapes[i].mesh.texcoords.begin(), shapes[i].mesh.texcoords.end());

			/* Concatenate the shape's indices to the new mesh
			 * Indices need to be incremented as we concatenate shapes */
			for (uint32_t j : shapes[i].mesh.indices) {
				indices.push_back(j + vertCount);
			}
			vertCount += int(shapes[i].mesh.positions.size()) / 3;
		}

		/* Optional resize and find min/max */
		_findMetaData_DEPRECATED(meshData, vertices, doResize);

		/* Upload */
		mesh->mPrimitiveType = GL_TRIANGLE_STRIP;
		if (vertices.size()) {
			mesh->addVertexBuffer_DEPRECATED(VertexType::Position, 0, 3, vertices);
		}
		if (normals.size()) {
			mesh->addVertexBuffer_DEPRECATED(VertexType::Normal, 1, 3, normals);
		}
		if (texCoords.size()) {
			mesh->addVertexBuffer_DEPRECATED(VertexType::Texture0, 2, 2, texCoords);
		}
		if (indices.size()) {
			mesh->mPrimitiveType = GL_TRIANGLES;
			mesh->addElementBuffer_DEPRECATED(indices);
		}

		NEO_LOG_I("Loaded mesh (%d vertices): %s", vertCount, fileName.c_str());
		return meshData;
	}

	std::vector<Asset_DEPRECATED> Loader::loadMultiAsset_DEPRECATED(const std::string &fileName) {
		/* If mesh was not found in map, read it in */
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> objMaterials;

		std::string resDir = APP_RES_DIR;
		if (!util::fileExists((resDir + fileName).c_str())) {
			resDir = ENGINE_RES_DIR;
			NEO_ASSERT(util::fileExists((resDir + fileName).c_str()), "Unable to find file: %s", fileName.c_str());
		}
		// TODO : use assimp or another optimized asset loader
		std::string errString;
		bool rc = tinyobj::LoadObj(shapes, objMaterials, errString, (resDir + fileName).c_str(), resDir.c_str());
		NEO_ASSERT(rc, errString.c_str());

		std::vector<Asset_DEPRECATED> ret;

		for (auto& shape : shapes) {
			Asset_DEPRECATED asset;

			Mesh* mesh = new Mesh;
			asset.meshData.mMesh = mesh;

			_findMetaData_DEPRECATED(asset.meshData, shape.mesh.positions, true);

			/* Upload */
			mesh->mPrimitiveType = GL_TRIANGLE_STRIP;
			if (shape.mesh.positions.size()) {
				mesh->addVertexBuffer_DEPRECATED(VertexType::Position, 0, 3, shape.mesh.positions);
			}
			if (shape.mesh.normals.size()) {
				mesh->addVertexBuffer_DEPRECATED(VertexType::Normal, 1, 3, shape.mesh.normals);
			}
			if (shape.mesh.texcoords.size()) {
				mesh->addVertexBuffer_DEPRECATED(VertexType::Texture0, 2, 2, shape.mesh.texcoords);
			}
			if (shape.mesh.indices.size()) {
				mesh->mPrimitiveType = GL_TRIANGLES;
				mesh->addElementBuffer_DEPRECATED(shape.mesh.indices);
			}

			NEO_LOG_I("Loaded mesh (%d vertices): %s of %s", shape.mesh.positions.size(), shape.name.c_str(), fileName.c_str());

			for (auto materialID : shape.mesh.material_ids) {
				if (materialID >= 0) {
					auto& material = objMaterials[materialID];
					asset.material.mAmbient = glm::vec3(material.ambient[0], material.ambient[1], material.ambient[2]);
					asset.material.mDiffuse = glm::vec3(material.diffuse[0], material.diffuse[1], material.diffuse[2]);
					asset.material.mSpecular = glm::vec3(material.specular[0], material.specular[1], material.specular[2]);
					asset.material.mShininess = material.shininess;

					TextureFormat format;
					format.mTarget = TextureTarget::Texture2D;
					format.mInternalFormat = GL_RGBA8;
					format.mBaseFormat = GL_RGBA;

					if (material.ambient_texname.size()) {
						// asset.material.mAmbient = Library::loadTexture(material.ambient_texname, format);
					}
					if (material.diffuse_texname.size()) {
						asset.material.mDiffuseMap = Library::loadTexture(material.diffuse_texname, format);
						asset.material.mDiffuseMap->genMips();
						// asset.material.mAlphaMap = asset.material.mDiffuseMap;
					}
					if (material.specular_texname.size()) {
						asset.material.mSpecularMap = Library::loadTexture(material.specular_texname, format);
						asset.material.mSpecularMap->genMips();
					}
					if (material.displacement_texname.size()) {
						asset.material.mNormalMap = Library::loadTexture(material.displacement_texname, format);
						asset.material.mNormalMap->genMips();
					}
					if (material.alpha_texname.size()) {
						asset.material.mAlphaMap = Library::loadTexture(material.alpha_texname, format);
						asset.material.mAlphaMap->genMips();
					}
				}
			}

			ret.push_back(asset);
		}

		return ret;
	}

	Texture* Loader::loadTexture(const std::string &fileName, TextureFormat format) {
		TRACY_ZONE();
		/* Create an empty texture if it is not already exist in the library */
		int width, height, components;
		uint8_t* data = _loadTextureData(width, height, components, fileName, format);

		Texture* texture = new Texture(format, glm::uvec2(width, height), data);

		_cleanTextureData(data);

		return texture;

	}

	Texture* Loader::loadTexture(const std::string &name, const std::vector<std::string>& files) {
		TRACY_ZONE();

		NEO_ASSERT(files.size() == 6, "Attempting to create cube map without 6 files");

		std::vector<uint8_t*> data;
		glm::u16vec2 size(UINT16_MAX, UINT16_MAX);
		for (int i = 0; i < 6; i++) {
			int _width, _height, _components;
			data.push_back(_loadTextureData(_width, _height, _components, files[i], {}, false));
			size.x = std::min(size.x, static_cast<uint16_t>(_width));
			size.y = std::min(size.y, static_cast<uint16_t>(_height));
		}

		/* Upload data to GPU and free from CPU */
		TextureFormat format = { TextureTarget::TextureCube, GL_RGBA, GL_RGBA, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_UNSIGNED_BYTE };
		Texture* texture = new Texture(format, size, reinterpret_cast<void**>(data.data()));

		/* Clean */
		for (int i = 0; i < 6; i++) {
			_cleanTextureData(data[i]);
		}

		NEO_LOG_I("Loaded cubemap (%s)", name.c_str());

		return texture;
	}

	uint8_t* Loader::_loadTextureData(int& width, int& height, int& components, const std::string& fileName, TextureFormat format, bool flip) {
		std::string _fileName = APP_RES_DIR + fileName;
		if (!util::fileExists(_fileName.c_str())) {
			_fileName = ENGINE_RES_DIR + fileName;
			NEO_ASSERT(util::fileExists(_fileName.c_str()), "Unable to find file %s", fileName.c_str());
		}

		/* Use stbi if name is an existing file */
		stbi_set_flip_vertically_on_load(flip);
		uint8_t *data = stbi_load(_fileName.c_str(), &width, &height, &components, format.mBaseFormat == GL_RGB ? STBI_rgb : STBI_rgb_alpha);
		NEO_ASSERT(data, "Error reading texture file");

		NEO_LOG_I("Loaded texture %s [%d, %d]", fileName.c_str(), width, height);

		return data;
	}

	void Loader::_cleanTextureData(uint8_t* data) {
		stbi_image_free(data);
	}

	/* Provided function to resize a mesh so all vertex positions are [0, 1.f] */
	void Loader::_findMetaData_DEPRECATED(MeshData& meshData, std::vector<float>& vertices, bool doResize) {
		float minX, minY, minZ;
		float maxX, maxY, maxZ;
		float scaleX, scaleY, scaleZ;
		float shiftX, shiftY, shiftZ;

		minX = minY = minZ = 1.1754E+38F;
		maxX = maxY = maxZ = -1.1754E+38F;

		//Go through all vertices to determine min and max of each dimension
		for (size_t v = 0; v < vertices.size() / 3; v++) {
			if (vertices[3 * v + 0] < minX) minX = vertices[3 * v + 0];
			if (vertices[3 * v + 0] > maxX) maxX = vertices[3 * v + 0];

			if (vertices[3 * v + 1] < minY) minY = vertices[3 * v + 1];
			if (vertices[3 * v + 1] > maxY) maxY = vertices[3 * v + 1];

			if (vertices[3 * v + 2] < minZ) minZ = vertices[3 * v + 2];
			if (vertices[3 * v + 2] > maxZ) maxZ = vertices[3 * v + 2];
		}

		//From min and max compute necessary scale and shift for each dimension
		float xExtent, yExtent, zExtent;
		xExtent = maxX - minX;
		yExtent = maxY - minY;
		zExtent = maxZ - minZ;

		float maxExtent = 0;
		if (xExtent >= yExtent && xExtent >= zExtent) {
			maxExtent = xExtent;
		}
		if (yExtent >= xExtent && yExtent >= zExtent) {
			maxExtent = yExtent;
		}
		if (zExtent >= xExtent && zExtent >= yExtent) {
			maxExtent = zExtent;
		}
		scaleX = 2.f / maxExtent;
		shiftX = minX + (xExtent / 2.f);
		scaleY = 2.f / maxExtent;
		shiftY = minY + (yExtent / 2.f);
		scaleZ = 2.f / maxExtent;
		shiftZ = minZ + (zExtent) / 2.f;

		if (doResize) {
			//Go through all verticies shift and scale them
			minX = minY = minZ = 1.1754E+38F;
			maxX = maxY = maxZ = -1.1754E+38F;
			for (size_t v = 0; v < vertices.size() / 3; v++) {
				vertices[3 * v + 0] = (vertices[3 * v + 0] - shiftX) * scaleX;
				minX = std::min(minX, vertices[3 * v + 0]);
				maxX = std::max(maxX, vertices[3 * v + 0]);
				assert(vertices[3 * v + 0] >= -1.0 - 0.001f);
				assert(vertices[3 * v + 0] <= 1.0 + 0.001f);
				vertices[3 * v + 1] = (vertices[3 * v + 1] - shiftY) * scaleY;
				minY = std::min(minY, vertices[3 * v + 1]);
				maxY = std::max(maxY, vertices[3 * v + 1]);
				assert(vertices[3 * v + 1] >= -1.0 - 0.001f);
				assert(vertices[3 * v + 1] <= 1.0 + 0.001f);
				vertices[3 * v + 2] = (vertices[3 * v + 2] - shiftZ) * scaleZ;
				minZ = std::min(minZ, vertices[3 * v + 2]);
				maxZ = std::max(maxZ, vertices[3 * v + 2]);
				assert(vertices[3 * v + 2] >= -1.0 - 0.001f);
				assert(vertices[3 * v + 2] <= 1.0 + 0.001f);
			}
		}

		meshData.mMin = glm::vec3(minX, minY, minZ);
		meshData.mMax = glm::vec3(maxX, maxY, maxZ);
		meshData.mBasePosition = glm::vec3(shiftX, shiftY, shiftZ);
		meshData.mBaseScale = glm::vec3(maxExtent) / 2.f;
	}

	Loader::GltfScene Loader::loadGltfScene(const std::string& fileName) {
		std::string _fileName = APP_RES_DIR + fileName;
		if (!util::fileExists(_fileName.c_str())) {
			_fileName = ENGINE_RES_DIR + fileName;
		}
		NEO_ASSERT(util::fileExists(_fileName.c_str()), "Unable to find file: %s after checking:\n\t%s\n\t%s\n", fileName.c_str(), APP_RES_DIR.c_str(), ENGINE_RES_DIR.c_str());

		tinygltf::Model model;
		tinygltf::TinyGLTF loader;
		std::string err;
		std::string warn;

		bool ret = false;
		if (fileName.substr(fileName.length() - 3, 3) == "glb") {
			ret = loader.LoadBinaryFromFile(&model, &err, &warn, _fileName.c_str());
		}
		else {
			ret = loader.LoadASCIIFromFile(&model, &err, &warn, _fileName.c_str());
		}

		if (!warn.empty()) {
			NEO_LOG_W("Warn: %s\n", warn.c_str());
		}

		if (!err.empty()) {
			NEO_LOG_E("Err: %s\n", err.c_str());
		}

		NEO_ASSERT(ret, "tinygltf failed to parse %s", _fileName.c_str());
		if (!ret) {
			return {};
		}

		// Translate tinygltf::Model to Loader::GltfScene
		NEO_LOG_V("Translating %s to neo", _fileName.c_str());
		if (model.lights.size()) {
			NEO_LOG_W("%s contains lights - ignoring", fileName.c_str());
		}
		if (model.cameras.size()) {
			NEO_LOG_W("%s contains cameras - ignoring", fileName.c_str());
		}
		if (model.defaultScene < 0 || model.scenes.size() > 1) {
			NEO_LOG_W("%s has weird scene layout. Just using the default or first one", fileName.c_str());
		}

		GltfScene outScene;
		for (const auto& nodeID : model.scenes[model.defaultScene].nodes) {
			const auto& node = model.nodes[nodeID];
			if (node.camera != -1 || node.light != -1 || node.mesh == -1) {
				continue;
			}

			GltfScene::MeshNode outNode;
			outNode.mName = node.name;

			// Spatial
			if (node.matrix.size() == 16) {
				outNode.mSpatial.setModelMatrix(glm::mat4x4(
					node.matrix[0],  node.matrix[1],  node.matrix[2],  node.matrix[3],
					node.matrix[4],  node.matrix[5],  node.matrix[6],  node.matrix[7],
					node.matrix[8],  node.matrix[9],  node.matrix[10], node.matrix[11],
					node.matrix[12], node.matrix[13], node.matrix[14], node.matrix[15]
				));
			}
			else {
				if (node.translation.size() == 3) {
					outNode.mSpatial.setPosition(glm::vec3(node.translation[0], node.translation[1], node.translation[2]));
				}
				if (node.scale.size() == 3) {
					outNode.mSpatial.setScale(glm::vec3(node.scale[0], node.scale[1], node.scale[2]));
				}
				NEO_ASSERT(node.rotation.empty(), "TODO: Handle quats");
			}

			// Mesh
			if (model.meshes[node.mesh].primitives.size() > 1) {
				NEO_LOG_W("Mesh has >1 mesh? Using the first...");
			}
			auto& gltfMesh = model.meshes[node.mesh].primitives[0];

			outNode.mMesh.mMesh = new Mesh;
			switch (gltfMesh.mode) {
			case TINYGLTF_MODE_POINTS:
				outNode.mMesh.mMesh->mPrimitiveType = GL_POINTS;
				break;
			case TINYGLTF_MODE_LINE:
				outNode.mMesh.mMesh->mPrimitiveType = GL_LINES;
				break;
			case TINYGLTF_MODE_LINE_LOOP:
				outNode.mMesh.mMesh->mPrimitiveType = GL_LINE_LOOP;
				break;
			case TINYGLTF_MODE_LINE_STRIP:
				outNode.mMesh.mMesh->mPrimitiveType = GL_LINE_STRIP;
				break;
			case TINYGLTF_MODE_TRIANGLES:
				outNode.mMesh.mMesh->mPrimitiveType = GL_TRIANGLES;
				break;
			case TINYGLTF_MODE_TRIANGLE_STRIP:
				outNode.mMesh.mMesh->mPrimitiveType = GL_TRIANGLE_STRIP;
				break;
			case TINYGLTF_MODE_TRIANGLE_FAN:
				outNode.mMesh.mMesh->mPrimitiveType = GL_TRIANGLE_FAN;
				break;
			}

			// Indices
			if (gltfMesh.indices > -1)
			{
				auto& accessor = model.accessors[gltfMesh.indices];
				NEO_ASSERT(!accessor.sparse.isSparse, "TODO : sparse?");
				auto& bufferView = model.bufferViews[accessor.bufferView];
				NEO_ASSERT(bufferView.target == TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER, "Not a index buffer?");
				const auto& buffer = model.buffers[bufferView.buffer];

				const uint8_t* bufferData = static_cast<const uint8_t*>(buffer.data.data()) + bufferView.byteOffset + accessor.byteOffset;
				outNode.mMesh.mMesh->addElementBuffer(
					static_cast<uint32_t>(accessor.count),
					static_cast<uint32_t>(_getGLFormat(accessor.componentType)),
					static_cast<uint32_t>(bufferView.byteLength), 
					bufferData
				);
			}

			for (const auto& attribute : gltfMesh.attributes) {
				const auto& accessor = model.accessors[attribute.second];

				VertexType vertexType = VertexType::Position;
				if (attribute.first == "POSITION") {
					if (accessor.maxValues.size() == 3) {
						outNode.mMesh.mMax = glm::vec3(accessor.maxValues[0], accessor.maxValues[1], accessor.maxValues[2]);
					}
					if (accessor.minValues.size() == 3) {
						outNode.mMesh.mMin = glm::vec3(accessor.minValues[0], accessor.minValues[1], accessor.minValues[2]);
					}
				}
				else if (attribute.first == "NORMAL") {
					vertexType = VertexType::Normal;
				}
				else {
					NEO_FAIL("TODO: unsupported attribute: %s", attribute.first.c_str());
				}

				NEO_ASSERT(!accessor.sparse.isSparse, "TODO: sparse");

				const auto& bufferView = model.bufferViews[accessor.bufferView];
				const auto& buffer = model.buffers[bufferView.buffer];

				const uint32_t stride = accessor.ByteStride(bufferView);
				const uint8_t* bufferData = static_cast<const uint8_t*>(buffer.data.data()) + bufferView.byteOffset;
				GLenum format = _getGLFormat(accessor.componentType);
				outNode.mMesh.mMesh->addVertexBuffer(
					vertexType, 
					tinygltf::GetNumComponentsInType(accessor.type), 
					stride, 
					format, 
					accessor.normalized, 
					static_cast<uint32_t>(accessor.count), 
					static_cast<uint32_t>(accessor.byteOffset), 
					static_cast<uint32_t>(bufferView.byteLength), 
					bufferData
				);
			}
			if (!model.meshes[node.mesh].name.empty()) {
				Library::insertMesh(model.meshes[node.mesh].name, outNode.mMesh);
			}

			// TODO: Materials
			if (gltfMesh.material > -1) {
				auto& material = model.materials[gltfMesh.material];

				if (!material.lods.empty()) {
					NEO_LOG_W("Material has LODs -- unsupported");

				}

				if (material.alphaMode == "OPAQUE") {
					outNode.mAlphaMode = GltfScene::MeshNode::AlphaMode::Opaque;
				}
				else if (material.alphaMode == "MASK") {
					outNode.mAlphaMode = GltfScene::MeshNode::AlphaMode::AlphaTest;
				}
				else if (material.alphaMode == "BLEND") {
					outNode.mAlphaMode = GltfScene::MeshNode::AlphaMode::Transparent;
					NEO_FAIL("Transparency is unsupported");
				}

				NEO_ASSERT(material.normalTexture.index == -1, "Normal maps unsupported");
				NEO_ASSERT(material.occlusionTexture.index == -1, "Occlusion maps unsupported");

				if (material.emissiveFactor.size() == 3) {
					outNode.mMaterial.mEmissive = glm::vec3(material.emissiveFactor[0], material.emissiveFactor[1], material.emissiveFactor[2]);
				}
				NEO_ASSERT(material.emissiveTexture.index == -1, "Emissive maps unsupported");

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
				NEO_ASSERT(material.pbrMetallicRoughness.baseColorTexture.index == -1, "Albedo maps unsupported");
				NEO_ASSERT(material.pbrMetallicRoughness.metallicRoughnessTexture.index == -1, "Metal/roughness maps unsupported");
			}

			// TODO: Textures/Images/Samplers

			outScene.mMeshNodes.push_back(outNode);

			if (!node.children.empty()) {
				NEO_FAIL("TODO: recurse children");
				NEO_FAIL("TODO: dont forget transofmration hierarchy");
			}
		}

		NEO_LOG_I("Successfully parsed %s", fileName.c_str());
		return outScene;
	}
}
