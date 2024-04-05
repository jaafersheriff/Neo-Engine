#pragma once

#include "ResourceManager/MeshResourceManager.hpp"
#include "ext/PerlinNoise.hpp"

namespace neo {

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
				uint32_t byteSize = static_cast<uint32_t>(verts.size() * sizeof(float));
				builder->mVertexBuffers[types::mesh::VertexType::Position] = {
					3,
					0,
					types::ByteFormats::Float,
					false,
					static_cast<uint32_t>(verts.size()),
					0,
					byteSize
				};
				builder->mVertexBuffers[types::mesh::VertexType::Position].mData = static_cast<uint8_t*>(malloc(byteSize));
				memcpy(const_cast<uint8_t*>(builder->mVertexBuffers[types::mesh::VertexType::Position].mData), verts.data(), byteSize);
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
				uint32_t byteSize = static_cast<uint32_t>(normals.size() * sizeof(float));
				builder->mVertexBuffers[types::mesh::VertexType::Normal] = {
					3,
					0,
					types::ByteFormats::Float,
					false,
					static_cast<uint32_t>(normals.size()),
					0,
					byteSize
				};
				builder->mVertexBuffers[types::mesh::VertexType::Normal].mData = static_cast<uint8_t*>(malloc(byteSize));
				memcpy(const_cast<uint8_t*>(builder->mVertexBuffers[types::mesh::VertexType::Normal].mData), normals.data(), byteSize);
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
				uint32_t byteSize = static_cast<uint32_t>(uvs.size() * sizeof(float));
				builder->mVertexBuffers[types::mesh::VertexType::Texture0] = {
					2,
					0,
					types::ByteFormats::Float,
					false,
					static_cast<uint32_t>(uvs.size()),
					0,
					byteSize
				};
				builder->mVertexBuffers[types::mesh::VertexType::Texture0].mData = static_cast<uint8_t*>(malloc(byteSize));
				memcpy(const_cast<uint8_t*>(builder->mVertexBuffers[types::mesh::VertexType::Texture0].mData), uvs.data(), byteSize);
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
				uint32_t byteSize = static_cast<uint32_t>(verts.size() * sizeof(float));
				builder->mVertexBuffers[types::mesh::VertexType::Position] = {
					3,
					0,
					types::ByteFormats::Float,
					false,
					static_cast<uint32_t>(verts.size()),
					0,
					byteSize
				};
				builder->mVertexBuffers[types::mesh::VertexType::Position].mData = static_cast<uint8_t*>(malloc(byteSize));
				memcpy(const_cast<uint8_t*>(builder->mVertexBuffers[types::mesh::VertexType::Position].mData), verts.data(), byteSize);
			}

			{
				std::vector<float> normals =
				{ 0.f, 0.f, 1.f,
				  0.f, 0.f, 1.f,
				  0.f, 0.f, 1.f,
				  0.f, 0.f, 1.f };
				uint32_t byteSize = static_cast<uint32_t>(normals.size() * sizeof(float));
				builder->mVertexBuffers[types::mesh::VertexType::Normal] = {
					3,
					0,
					types::ByteFormats::Float,
					false,
					static_cast<uint32_t>(normals.size()),
					0,
					byteSize
				};
				builder->mVertexBuffers[types::mesh::VertexType::Normal].mData = static_cast<uint8_t*>(malloc(byteSize));
				memcpy(const_cast<uint8_t*>(builder->mVertexBuffers[types::mesh::VertexType::Normal].mData), normals.data(), byteSize);
			}

			{
				std::vector<float> uvs =
				{ 0.f, 0.f,
				  1.f, 0.f,
				  0.f, 1.f,
				  1.f, 1.f };
				uint32_t byteSize = static_cast<uint32_t>(uvs.size() * sizeof(float));
				builder->mVertexBuffers[types::mesh::VertexType::Texture0] = {
					2,
					0,
					types::ByteFormats::Float,
					false,
					static_cast<uint32_t>(uvs.size()),
					0,
					byteSize
				};
				builder->mVertexBuffers[types::mesh::VertexType::Texture0].mData = static_cast<uint8_t*>(malloc(byteSize));
				memcpy(const_cast<uint8_t*>(builder->mVertexBuffers[types::mesh::VertexType::Texture0].mData), uvs.data(), byteSize);
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

			float t = (float)(1.f + (glm::sqrt(5.0)) / 2.f);
			float length = glm::length(glm::vec3(1, 0, t));
			std::vector<float> verts = {
				 -0.5f / length,    t / length,  0.f / length,
				  0.5f / length,    t / length,  0.f / length,
				 -0.5f / length,   -t / length,  0.f / length,
				  0.5f / length,   -t / length,  0.f / length,
				  0.f / length, -0.5f / length,	   t / length,
				  0.f / length,  0.5f / length,	   t / length,
				  0.f / length, -0.5f / length,   -t / length,
				  0.f / length,  0.5f / length,   -t / length,
					t / length,  0.f / length, -0.5f / length,
					t / length,  0.f / length,  0.5f / length,
				   -t / length,  0.f / length, -0.5f / length,
				   -t / length,  0.f / length,  0.5f / length
			};
			NEO_LOG_V("Min: %0.3f -- max: %0.3f", -t / length, t / length);

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
					glm::vec3 halfA = glm::normalize((v1 + v2) / 2.f);
					glm::vec3 halfB = glm::normalize((v2 + v3) / 2.f);
					glm::vec3 halfC = glm::normalize((v3 + v1) / 2.f);
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
				builder->mVertexBuffers[types::mesh::VertexType::Position] = {
					3,
					0,
					types::ByteFormats::Float,
					false,
					static_cast<uint32_t>(verts.size()),
					0,
					byteSize
				};
				builder->mVertexBuffers[types::mesh::VertexType::Position].mData = static_cast<uint8_t*>(malloc(byteSize));
				memcpy(const_cast<uint8_t*>(builder->mVertexBuffers[types::mesh::VertexType::Position].mData), verts.data(), byteSize);

				builder->mVertexBuffers[types::mesh::VertexType::Normal] = builder->mVertexBuffers[types::mesh::VertexType::Position];
				builder->mVertexBuffers[types::mesh::VertexType::Normal].mData = static_cast<uint8_t*>(malloc(byteSize));
				memcpy(const_cast<uint8_t*>(builder->mVertexBuffers[types::mesh::VertexType::Normal].mData), verts.data(), byteSize);
			}
			{
				uint32_t byteSize = static_cast<uint32_t>(tex.size() * sizeof(float));
				builder->mVertexBuffers[types::mesh::VertexType::Texture0] = {
					2,
					0,
					types::ByteFormats::Float,
					false,
					static_cast<uint32_t>(tex.size()),
					0,
					byteSize
				};
				builder->mVertexBuffers[types::mesh::VertexType::Texture0].mData = static_cast<uint8_t*>(malloc(byteSize));
				memcpy(const_cast<uint8_t*>(builder->mVertexBuffers[types::mesh::VertexType::Texture0].mData), tex.data(), byteSize);
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
	}
}
