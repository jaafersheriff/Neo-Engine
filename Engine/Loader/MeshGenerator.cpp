#pragma once

#include "ResourceManager/MeshManager.hpp"

#include "ext/PerlinNoise.hpp"
#include "ext/par/par_shapes.h"

namespace neo {
	namespace {
		void _uploadFloatBuffer(MeshLoadDetails::VertexBuffer& buffer, uint32_t components, uint32_t elements, uint32_t byteSize, uint8_t* data) {
			buffer.mComponents = components;
			buffer.mStride = 0;
			buffer.mFormat = types::ByteFormats::Float;
			buffer.mNormalized = false;
			buffer.mCount = elements;
			buffer.mOffset = 0;
			buffer.mByteSize = byteSize;
			buffer.mData = static_cast<uint8_t*>(malloc(byteSize));
			if (buffer.mData) {
				memcpy(const_cast<uint8_t*>(buffer.mData), data, byteSize);
			}
		}

		void _uploadParShape(MeshLoadDetails& details, par_shapes_mesh* mesh) {
			if (!mesh || !mesh->points || !mesh->triangles) {
				return;
			}
			// TODO - scale it so it fits within -0.5, 0.5
			par_shapes_unweld(mesh, true);
			par_shapes_compute_normals(mesh);

			details.mPrimtive = types::mesh::Primitive::Triangles;
			_uploadFloatBuffer(
				details.mVertexBuffers[types::mesh::VertexType::Position],
				3,
				static_cast<uint32_t>(mesh->npoints * 3),
				static_cast<uint32_t>(mesh->npoints * 3 * sizeof(float)),
				reinterpret_cast<uint8_t*>(mesh->points)
			);
			if (mesh->normals) {
				_uploadFloatBuffer(
					details.mVertexBuffers[types::mesh::VertexType::Normal],
					3,
					static_cast<uint32_t>(mesh->npoints * 3),
					static_cast<uint32_t>(mesh->npoints * 3 * sizeof(float)),
					reinterpret_cast<uint8_t*>(mesh->normals)
				);
			}
			{
				uint32_t byteSize = static_cast<uint32_t>(mesh->ntriangles) * sizeof(PAR_SHAPES_T) * 3u;
				details.mElementBuffer = {
					static_cast<uint32_t>(mesh->ntriangles * 3),
					types::ByteFormats::UnsignedShort,
					byteSize
				};
				details.mElementBuffer->mData = static_cast<uint8_t*>(malloc(byteSize));
				memcpy(const_cast<uint8_t*>(details.mElementBuffer->mData), mesh->triangles, byteSize);
			}

			par_shapes_free_mesh(mesh);
		}

	}

	namespace prefabs {

		std::unique_ptr<MeshLoadDetails> generateCube() {
			std::unique_ptr<MeshLoadDetails> builder = std::make_unique<MeshLoadDetails>();
			builder->mPrimtive = types::mesh::Primitive::Triangles;

			{
				std::vector<float> verts =
				{ -0.5f, -0.5f, -0.5f,
				  0.5f,  0.5f, -0.5f,
				  0.5f, -0.5f, -0.5f,
				 -0.5f,  0.5f, -0.5f,
				 -0.5f, -0.5f, -0.5f,
				 -0.5f,  0.5f,  0.5f,
				 -0.5f,  0.5f, -0.5f,
				 -0.5f, -0.5f,  0.5f,
				 -0.5f,  0.5f, -0.5f,
				  0.5f,  0.5f,  0.5f,
				  0.5f,  0.5f, -0.5f,
				 -0.5f,  0.5f,  0.5f,
				  0.5f, -0.5f, -0.5f,
				  0.5f,  0.5f, -0.5f,
				  0.5f,  0.5f,  0.5f,
				  0.5f, -0.5f,  0.5f,
				 -0.5f, -0.5f, -0.5f,
				  0.5f, -0.5f, -0.5f,
				  0.5f, -0.5f,  0.5f,
				 -0.5f, -0.5f,  0.5f,
				 -0.5f, -0.5f,  0.5f,
				  0.5f, -0.5f,  0.5f,
				  0.5f,  0.5f,  0.5f,
				 -0.5f,  0.5f,  0.5f };
				_uploadFloatBuffer(
					builder->mVertexBuffers[types::mesh::VertexType::Position], 
					3, 
					static_cast<uint32_t>(verts.size()), 
					static_cast<uint32_t>(verts.size() * sizeof(float)), 
					reinterpret_cast<uint8_t*>(verts.data())
				);
			}

			{
				std::vector<float> normals =
				{ 0,  0, -1,
				  0,  0, -1,
				  0,  0, -1,
				  0,  0, -1,
				 -1,  0,  0,
				 -1,  0,  0,
				 -1,  0,  0,
				 -1,  0,  0,
				  0,  1,  0,
				  0,  1,  0,
				  0,  1,  0,
				  0,  1,  0,
				  1,  0,  0,
				  1,  0,  0,
				  1,  0,  0,
				  1,  0,  0,
				  0, -1,  0,
				  0, -1,  0,
				  0, -1,  0,
				  0, -1,  0,
				  0,  0,  1,
				  0,  0,  1,
				  0,  0,  1,
				  0,  0,  1 };
				_uploadFloatBuffer(
					builder->mVertexBuffers[types::mesh::VertexType::Normal], 
					3, 
					static_cast<uint32_t>(normals.size()), 
					static_cast<uint32_t>(normals.size() * sizeof(float)), 
					reinterpret_cast<uint8_t*>(normals.data())
				);
			}

			{
				std::vector<float> uvs =
				{ 1.f, 0.f,
					0.f, 1.f,
					0.f, 0.f,
					1.f, 1.f,

					0.f, 0.f,
					1.f, 1.f,
					0.f, 1.f,
					1.f, 0.f,

					1.f, 0.f,
					0.f, 1.f,
					0.f, 0.f,
					1.f, 1.f,

					1.f, 0.f,
					1.f, 1.f,
					0.f, 1.f,
					0.f, 0.f,

					1.f, 1.f,
					0.f, 1.f,
					0.f, 0.f,
					1.f, 0.f,

					0.f, 0.f,
					1.f, 0.f,
					1.f, 1.f,
					0.f, 1.f };
				_uploadFloatBuffer(
					builder->mVertexBuffers[types::mesh::VertexType::Texture0], 
					2, 
					static_cast<uint32_t>(uvs.size()), 
					static_cast<uint32_t>(uvs.size() * sizeof(float)), 
					reinterpret_cast<uint8_t*>(uvs.data())
				);
			}

			{
				std::vector<uint32_t> indices =
				{ 0,  1,  2,
				  0,  3,  1,
				  4,  5,  6,
				  4,  7,  5,
				  8,  9, 10,
				  8, 11,  9,
				 12, 13, 14,
				 12, 14, 15,
				 16, 17, 18,
				 16, 18, 19,
				 20, 21, 22,
				 20, 22, 23 };
				uint32_t byteSize = static_cast<uint32_t>(indices.size() * sizeof(uint32_t));
				builder->mElementBuffer = {
					static_cast<uint32_t>(indices.size()),
					types::ByteFormats::UnsignedInt,
					byteSize
				};
				builder->mElementBuffer->mData = static_cast<uint8_t*>(malloc(byteSize));
				memcpy(const_cast<uint8_t*>(builder->mElementBuffer->mData), indices.data(), byteSize);
			}

			return std::move(builder);
		}

		std::unique_ptr<MeshLoadDetails> generateQuad() {
			auto builder = std::make_unique<MeshLoadDetails>();
			builder->mPrimtive = types::mesh::Primitive::Triangles;

			{
				std::vector<float> verts =
				{ -0.5f, -0.5f,  0.f,
				   0.5f, -0.5f,  0.f,
				  -0.5f,  0.5f,  0.f,
				   0.5f,  0.5f,  0.f };
				_uploadFloatBuffer(
					builder->mVertexBuffers[types::mesh::VertexType::Position], 
					3, 
					static_cast<uint32_t>(verts.size()), 
					static_cast<uint32_t>(verts.size() * sizeof(float)), 
					reinterpret_cast<uint8_t*>(verts.data())
				);
			}

			{
				std::vector<float> normals =
				{ 0.f, 0.f, 1.f,
				  0.f, 0.f, 1.f,
				  0.f, 0.f, 1.f,
				  0.f, 0.f, 1.f };
				_uploadFloatBuffer(
					builder->mVertexBuffers[types::mesh::VertexType::Normal], 
					3, 
					static_cast<uint32_t>(normals.size()), 
					static_cast<uint32_t>(normals.size() * sizeof(float)), 
					reinterpret_cast<uint8_t*>(normals.data())
				);
			}

			{
				std::vector<float> uvs =
				{ 0.f, 0.f,
				  1.f, 0.f,
				  0.f, 1.f,
				  1.f, 1.f };
				_uploadFloatBuffer(
					builder->mVertexBuffers[types::mesh::VertexType::Texture0], 
					2, 
					static_cast<uint32_t>(uvs.size()), 
					static_cast<uint32_t>(uvs.size() * sizeof(float)), 
					reinterpret_cast<uint8_t*>(uvs.data())
				);
			}

			{
				std::vector<uint32_t> indices =
				{ 0, 1, 2,
				  1, 3, 2 };
				uint32_t byteSize = static_cast<uint32_t>(indices.size() * sizeof(uint32_t));
				builder->mElementBuffer = {
					static_cast<uint32_t>(indices.size()),
					types::ByteFormats::UnsignedInt,
					byteSize
				};
				builder->mElementBuffer->mData = static_cast<uint8_t*>(malloc(byteSize));
				memcpy(const_cast<uint8_t*>(builder->mElementBuffer->mData), indices.data(), byteSize);
			}

			return std::move(builder);
		}

		// http://blog.andreaskahler.com/2009/06/creating-icosphere-mesh-in-code.html
		std::unique_ptr<MeshLoadDetails> generateSphere(int recursions) {
			std::unique_ptr<MeshLoadDetails> builder = std::make_unique<MeshLoadDetails>();
			builder->mPrimtive = types::mesh::Primitive::Triangles;

			float t = (1.f + glm::sqrt(5.f)) / 2.f;
			float length = glm::length(glm::vec3(0.5f, 0, t));
			length *= 2.f; // Scale [-1, 1] bounds to [-0.5, 0.5]
			std::vector<float> verts = {
				 -0.5f / length,    t / length,  0.f / length,
				  0.5f / length,    t / length,  0.f / length,
				 -0.5f / length,   -t / length,  0.f / length,
				  0.5f / length,   -t / length,  0.f / length,
				  0.f / length, -0.5f / length,	  t / length,
				  0.f / length,  0.5f / length,	  t / length,
				  0.f / length, -0.5f / length,   -t / length,
				  0.f / length,  0.5f / length,   -t / length,
					t / length,  0.f / length, -0.5f / length,
					t / length,  0.f / length,  0.5f / length,
				   -t / length,  0.f / length, -0.5f / length,
				   -t / length,  0.f / length,  0.5f / length
			};

			std::vector<uint32_t> ele = {
				  0, 11,  5,
				  0,  5,  1,
				  0,  1,  7,
				  0,  7, 10,
				  0, 10, 11,
				  1,  5,  9,
				  5, 11,  4,
				 11, 10,  2,
				 10,  7,  6,
				  7,  1,  8,
				  3,  9,  4,
				  3,  4,  2,
				  3,  2,  6,
				  3,  6,  8,
				  3,  8,  9,
				  4,  9,  5,
				  2,  4, 11,
				  6,  2, 10,
				  8,  6,  7,
				  9,  8,  1,
			};

			for (int i = 1; i <= recursions; i++) {
				std::vector<uint32_t> ele2;
				for (unsigned j = 0; j <= ele.size() - 3; j += 3) {
					// find 3 verts of old face
					glm::vec3 v1(verts[3 * ele[j + 0] + 0], verts[3 * ele[j + 0] + 1], verts[3 * ele[j + 0] + 2]);
					glm::vec3 v2(verts[3 * ele[j + 1] + 0], verts[3 * ele[j + 1] + 1], verts[3 * ele[j + 1] + 2]);
					glm::vec3 v3(verts[3 * ele[j + 2] + 0], verts[3 * ele[j + 2] + 1], verts[3 * ele[j + 2] + 2]);

					// add verts of new tris
					// Also scale [-1, 1] to [-0.5, 0.5]
					glm::vec3 halfA = glm::normalize((v1 + v2) / 2.f) / 2.f;
					glm::vec3 halfB = glm::normalize((v2 + v3) / 2.f) / 2.f;
					glm::vec3 halfC = glm::normalize((v3 + v1) / 2.f) / 2.f;
					verts.push_back(halfA.x);
					verts.push_back(halfA.y);
					verts.push_back(halfA.z);
					verts.push_back(halfB.x);
					verts.push_back(halfB.y);
					verts.push_back(halfB.z);
					verts.push_back(halfC.x);
					verts.push_back(halfC.y);
					verts.push_back(halfC.z);

					// add indices of new faces 
					uint32_t indA = static_cast<uint32_t>(verts.size()) / 3 - 3;
					uint32_t indB = static_cast<uint32_t>(verts.size()) / 3 - 2;
					uint32_t indC = static_cast<uint32_t>(verts.size()) / 3 - 1;
					ele2.push_back(ele[j + 0]);
					ele2.push_back(indA);
					ele2.push_back(indC);
					ele2.push_back(ele[j + 1]);
					ele2.push_back(indB);
					ele2.push_back(indA);
					ele2.push_back(ele[j + 2]);
					ele2.push_back(indC);
					ele2.push_back(indB);
					ele2.push_back(indA);
					ele2.push_back(indB);
					ele2.push_back(indC);
				}

				ele = ele2;
			}

			// calculate UV coords
			std::vector<float> tex;
			for (unsigned i = 0; i < verts.size(); i += 3) {
				tex.push_back(glm::clamp(0.5f + std::atan2(verts[i + 2], verts[i]) / (2.f * util::PI), 0.f, 1.f));
				tex.push_back(glm::clamp(0.5f + std::asin(verts[i + 1]) / util::PI, 0.f, 1.f));
			}

			{
				uint32_t byteSize = static_cast<uint32_t>(verts.size() * sizeof(float));
				_uploadFloatBuffer(
					builder->mVertexBuffers[types::mesh::VertexType::Position], 
					3, 
					static_cast<uint32_t>(verts.size()), 
					byteSize,
					reinterpret_cast<uint8_t*>(verts.data())
				);
				_uploadFloatBuffer(
					builder->mVertexBuffers[types::mesh::VertexType::Normal], 
					3, 
					static_cast<uint32_t>(verts.size()), 
					byteSize,
					reinterpret_cast<uint8_t*>(verts.data())
				);
			}
			{
				_uploadFloatBuffer(
					builder->mVertexBuffers[types::mesh::VertexType::Texture0], 
					2, 
					static_cast<uint32_t>(tex.size()), 
					static_cast<uint32_t>(tex.size() * sizeof(float)), 
					reinterpret_cast<uint8_t*>(tex.data())
				);
			}
			{
				uint32_t byteSize = static_cast<uint32_t>(ele.size() * sizeof(uint32_t));
				builder->mElementBuffer = {
					static_cast<uint32_t>(ele.size()),
					types::ByteFormats::UnsignedInt,
					byteSize
				};
				builder->mElementBuffer->mData = static_cast<uint8_t*>(malloc(byteSize));
				memcpy(const_cast<uint8_t*>(builder->mElementBuffer->mData), ele.data(), byteSize);
			}

			return std::move(builder);
		}

		[[nodiscard]] std::unique_ptr<MeshLoadDetails> generateIcosahedron() {
			std::unique_ptr<MeshLoadDetails> builder = std::make_unique<MeshLoadDetails>();
			_uploadParShape(*builder, par_shapes_create_icosahedron());
			return std::move(builder);
		}
		[[nodiscard]] std::unique_ptr<MeshLoadDetails> generateOctahedron() {
			std::unique_ptr<MeshLoadDetails> builder = std::make_unique<MeshLoadDetails>();
			_uploadParShape(*builder, par_shapes_create_octahedron());
			return std::move(builder);
		}
		[[nodiscard]] std::unique_ptr<MeshLoadDetails> generateTetrahedron() {
			std::unique_ptr<MeshLoadDetails> builder = std::make_unique<MeshLoadDetails>();
			_uploadParShape(*builder, par_shapes_create_tetrahedron());
			return std::move(builder);

		}

	}
}
