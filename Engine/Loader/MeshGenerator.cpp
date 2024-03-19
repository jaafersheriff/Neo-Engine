#pragma once

#include "ResourceManager/MeshResourceManager.hpp"
#include "ext/PerlinNoise.hpp"

namespace neo {

	namespace prefabs {

		void generateCube(HashedString id, MeshManager& meshManager) {
			MeshManager::MeshBuilder builder;

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
			builder.mVertexBuffers[types::mesh::VertexType::Position] = {
				3,
				0,
				types::ByteFormats::Float,
				false,
				static_cast<uint32_t>(verts.size()),
				0,
				static_cast<uint32_t>(verts.size() * sizeof(float)),
				reinterpret_cast<uint8_t*>(verts.data())
			};
			builder.mMin = glm::vec3(-0.5f);
			builder.mMax = glm::vec3(0.5f);

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
			builder.mVertexBuffers[types::mesh::VertexType::Normal] = {
				3, 
				0, 
				types::ByteFormats::Float,
				false,
				static_cast<uint32_t>(normals.size()),
				0,
				static_cast<uint32_t>(normals.size() * sizeof(float)),
				reinterpret_cast<uint8_t*>(normals.data())
			};

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
			builder.mVertexBuffers[types::mesh::VertexType::Texture0] = {
				2, 
				0, 
				types::ByteFormats::Float,
				false,
				static_cast<uint32_t>(uvs.size()),
				0,
				static_cast<uint32_t>(uvs.size() * sizeof(float)),
				reinterpret_cast<uint8_t*>(uvs.data())
			};

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
			builder.mElementBuffer = {
				static_cast<uint32_t>(indices.size()),
				types::ByteFormats::UnsignedInt,
				static_cast<uint32_t>(indices.size() * sizeof(uint32_t)),
				reinterpret_cast<uint8_t*>(indices.data())
			};

			builder.mPrimtive = types::mesh::Primitive::Triangles;

			auto _id = meshManager.load(id, builder);
			NEO_UNUSED(_id);
		}

		void generateQuad(HashedString id, MeshManager& meshManager) {
			MeshManager::MeshBuilder builder;

			std::vector<float> verts =
			{ -0.5f, -0.5f,  0.f,
			   0.5f, -0.5f,  0.f,
			  -0.5f,  0.5f,  0.f,
			   0.5f,  0.5f,  0.f };
			builder.mVertexBuffers[types::mesh::VertexType::Position] = {
				3, 
				0, 
				types::ByteFormats::Float,
				false,
				static_cast<uint32_t>(verts.size()),
				0,
				static_cast<uint32_t>(verts.size() * sizeof(float)),
				reinterpret_cast<uint8_t*>(verts.data())
			};
			builder.mMin = glm::vec3(-0.5f, -0.5f, -0.1f);
			builder.mMax = glm::vec3(0.5f, 0.5f, 0.1f);

			std::vector<float> normals =
			{ 0.f, 0.f, 1.f,
			  0.f, 0.f, 1.f,
			  0.f, 0.f, 1.f,
			  0.f, 0.f, 1.f };
			builder.mVertexBuffers[types::mesh::VertexType::Normal] = {
				3,
				0,
				types::ByteFormats::Float,
				false,
				static_cast<uint32_t>(normals.size()),
				0,
				static_cast<uint32_t>(normals.size() * sizeof(float)),
				reinterpret_cast<uint8_t*>(normals.data())
			};

			std::vector<float> uvs =
			{ 0.f, 0.f,
			  1.f, 0.f,
			  0.f, 1.f,
			  1.f, 1.f };
			builder.mVertexBuffers[types::mesh::VertexType::Texture0] = {
				2,
				0,
				types::ByteFormats::Float,
				false,
				static_cast<uint32_t>(uvs.size()),
				0,
				static_cast<uint32_t>(uvs.size() * sizeof(float)),
				reinterpret_cast<uint8_t*>(uvs.data())
			};

			std::vector<uint32_t> indices =
			{ 0, 1, 2,
			  1, 3, 2 };
			builder.mElementBuffer = {
				static_cast<uint32_t>(indices.size()),
				types::ByteFormats::UnsignedInt,
				static_cast<uint32_t>(indices.size() * sizeof(uint32_t)),
				reinterpret_cast<uint8_t*>(indices.data())
			};

			builder.mPrimtive = types::mesh::Primitive::Triangles;

			auto _id = meshManager.load(id, builder);
			NEO_UNUSED(_id);
		}

		// http://blog.andreaskahler.com/2009/06/creating-icosphere-mesh-in-code.html
		void generateSphere(HashedString id, MeshManager& meshManager, int recursions) {
			MeshManager::MeshBuilder builder;

			float t = (float)(1.f + (glm::sqrt(5.0)) / 2.f);
			float length = glm::length(glm::vec3(1, 0, t));
			std::vector<float> verts = {
				 -1.f / length,	t / length,  0.f / length,
				  1.f / length,	t / length,  0.f / length,
				 -1.f / length,   -t / length,  0.f / length,
				  1.f / length,   -t / length,  0.f / length,
				  0.f / length, -1.f / length,	t / length,
				  0.f / length,  1.f / length,	t / length,
				  0.f / length, -1.f / length,   -t / length,
				  0.f / length,  1.f / length,   -t / length,
					t / length,  0.f / length, -1.f / length,
					t / length,  0.f / length,  1.f / length,
				   -t / length,  0.f / length, -1.f / length,
				   -t / length,  0.f / length,  1.f / length
			};
			builder.mMin = glm::vec3(-t / length);
			builder.mMax = glm::vec3(t / length);

			std::vector<unsigned> ele = {
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
					builder.mMin = glm::min(builder.mMin, glm::min(halfA, glm::min(halfB, halfC)));
					builder.mMax = glm::max(builder.mMax, glm::max(halfA, glm::max(halfB, halfC)));

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

			builder.mVertexBuffers[types::mesh::VertexType::Position] = {
				3,
				0,
				types::ByteFormats::Float,
				false,
				static_cast<uint32_t>(verts.size()),
				0,
				static_cast<uint32_t>(verts.size() * sizeof(float)),
				reinterpret_cast<uint8_t*>(verts.data())
			};
			builder.mVertexBuffers[types::mesh::VertexType::Normal] = {
				3, 
				0, 
				types::ByteFormats::Float,
				false,
				static_cast<uint32_t>(verts.size()),
				0,
				static_cast<uint32_t>(verts.size() * sizeof(float)),
				reinterpret_cast<uint8_t*>(verts.data())
			};
			builder.mVertexBuffers[types::mesh::VertexType::Texture0] = {
				2, 
				0, 
				types::ByteFormats::Float,
				false,
				static_cast<uint32_t>(tex.size()),
				0,
				static_cast<uint32_t>(tex.size() * sizeof(float)),
				reinterpret_cast<uint8_t*>(tex.data())
			};
			builder.mElementBuffer = {
				static_cast<uint32_t>(ele.size()),
				types::ByteFormats::UnsignedInt,
				static_cast<uint32_t>(ele.size() * sizeof(uint32_t)),
				reinterpret_cast<uint8_t*>(ele.data())
			};

			builder.mPrimtive = types::mesh::Primitive::Triangles;

			auto _id = meshManager.load(id, builder);
			NEO_UNUSED(_id);
		}

		Mesh generatePlane(float h, int VERTEX_COUNT, int numOctaves) {
			siv::PerlinNoise noise(rand());

			Mesh mesh = Mesh();
			mesh.mMin = glm::vec3(FLT_MAX);
			mesh.mMax = glm::vec3(FLT_MIN);

			int count = VERTEX_COUNT * VERTEX_COUNT;

			std::vector<std::vector<float>> heights;
			heights.resize(VERTEX_COUNT);
			for (int i = 0; i < VERTEX_COUNT; i++) {
				heights[i].resize(VERTEX_COUNT);
			}

			std::vector<float> vertices;
			vertices.resize(count * 3);

			std::vector<float> normals;
			normals.resize(count * 3);

			std::vector<float> textureCoords;
			textureCoords.resize(count * 2);

			std::vector<uint32_t> indices;
			indices.resize(6 * (VERTEX_COUNT - 1) * (VERTEX_COUNT * 1));

			for (int i = 0; i < VERTEX_COUNT; i++) {
				for (int j = 0; j < VERTEX_COUNT; j++) {
					heights[j][i] = h == 0.f ? h : h * static_cast<float>(noise.octaveNoise(j / static_cast<double>(VERTEX_COUNT), i / static_cast<double>(VERTEX_COUNT), numOctaves));
				}
			}

			int vertexPointer = 0;
			for (int i = 0; i < VERTEX_COUNT; i++) {
				for (int j = 0; j < VERTEX_COUNT; j++) {

					glm::vec3 vert = glm::vec3(
						(float)j / ((float)VERTEX_COUNT - 1),
						heights[j][i],
						(float)i / ((float)VERTEX_COUNT - 1));

					vertices[vertexPointer * 3 + 0] = vert.x;
					vertices[vertexPointer * 3 + 1] = vert.y;
					vertices[vertexPointer * 3 + 2] = vert.z;

					float heightL = (h == 0.f || j == 0) ? 0.f : heights[j - 1][i] / h;
					float heightR = (h == 0.f || j == VERTEX_COUNT - 1) ? 0.f : heights[j + 1][i] / h;
					float heightD = (h == 0.f || i == VERTEX_COUNT - 1) ? 0.f : heights[j][i + 1] / h;
					float heightU = (h == 0.f || i == 0) ? 0.f : heights[j][i - 1] / h;
					glm::vec3 normal = glm::normalize(glm::vec3(heightL - heightR, 2.f / ((float)VERTEX_COUNT - 1), heightD - heightU));
					normals[vertexPointer * 3 + 0] = normal.x;
					normals[vertexPointer * 3 + 1] = normal.y;
					normals[vertexPointer * 3 + 2] = normal.z;

					textureCoords[vertexPointer * 2] = (float)j / ((float)VERTEX_COUNT - 1);
					textureCoords[vertexPointer * 2 + 1] = (float)i / ((float)VERTEX_COUNT - 1);

					mesh.mMin = glm::min(mesh.mMin, vert);
					mesh.mMax = glm::max(mesh.mMax, vert);
					vertexPointer++;
				}
			}

			int indexPointer = 0;
			for (int i = 0; i < VERTEX_COUNT - 1; i++) {
				for (int j = 0; j < VERTEX_COUNT - 1; j++) {

					uint32_t topLeft = (i * VERTEX_COUNT) + j;
					uint32_t topRight = topLeft + 1;
					uint32_t bottomLeft = ((i + 1) * VERTEX_COUNT) + j;
					uint32_t bottomRight = bottomLeft + 1;
					indices[indexPointer++] = topLeft;
					indices[indexPointer++] = bottomLeft;
					indices[indexPointer++] = topRight;
					indices[indexPointer++] = topRight;
					indices[indexPointer++] = bottomLeft;
					indices[indexPointer++] = bottomRight;
				}
			}

			mesh.addVertexBuffer(
				types::mesh::VertexType::Position, 
				3, 
				0, 
				types::ByteFormats::Float,
				false,
				static_cast<uint32_t>(vertices.size()),
				0,
				static_cast<uint32_t>(vertices.size() * sizeof(float)),
				reinterpret_cast<uint8_t*>(vertices.data())
			);
			mesh.addVertexBuffer(
				types::mesh::VertexType::Normal, 
				3, 
				0, 
				types::ByteFormats::Float,
				false,
				static_cast<uint32_t>(normals.size()),
				0,
				static_cast<uint32_t>(normals.size() * sizeof(float)),
				reinterpret_cast<uint8_t*>(normals.data())
			);

			mesh.addVertexBuffer(
				types::mesh::VertexType::Texture0, 
				2, 
				0, 
				types::ByteFormats::Float,
				false,
				static_cast<uint32_t>(textureCoords.size()),
				0,
				static_cast<uint32_t>(textureCoords.size() * sizeof(float)),
				reinterpret_cast<uint8_t*>(textureCoords.data())
			);
			mesh.addElementBuffer(
				static_cast<uint32_t>(indices.size()),
				types::ByteFormats::UnsignedInt,
				static_cast<uint32_t>(indices.size() * sizeof(uint32_t)),
				reinterpret_cast<uint8_t*>(indices.data())
			);


			mesh.mPrimitiveType = types::mesh::Primitive::Triangles;

			return mesh;
		}
	}
}
