#pragma once

#include "ECS/Systems/System.hpp"
#include "Engine.hpp"

#include "MetaballComponent.hpp"
#include "MetaballsMeshComponent.hpp"

using namespace neo;

class MetaballsSystem : public System {

    public:
        struct Grid {
            Grid() : 
                mVal(0.0f) {
                mNormal = glm::vec3(0.f);
            }

            float mVal;
            glm::vec3 mNormal;
        };

        int mDims = 32;
        bool mAutoUpdate = true;
        bool mDirtyBalls = true;

        MetaballsSystem() :
            System("Metaballs System") {
        }

        virtual void init() override {
            update(0.f);
        }

        virtual void imguiEditor() override {
            ImGui::Checkbox("Auto update", &mAutoUpdate);
        }

        virtual void update(const float dt) override {
            auto metaballMesh = Engine::getSingleComponent<MetaballsMeshComponent>();
            if (!metaballMesh) {
                return;
            }

            auto balls = Engine::getComponentTuples<MetaballComponent, SpatialComponent>();
            if (balls.empty()) {
                return;
            }

            if (mAutoUpdate) {
                MICROPROFILE_ENTERI("Metaballs System", "updatePositions", MP_AUTO);
                auto rTime = Util::getRunTime();
                for (uint32_t ii = 0; ii < balls.size(); ++ii) {
                    auto spatial = balls[ii]->get<SpatialComponent>();
                    glm::vec3 position = spatial->getPosition();
                    float radius = spatial->getScale().x;

                    position.x = glm::sin(rTime*(ii*0.21f) + ii * 0.37f) * (mDims * 0.5f - 8.0f);
                    position.y = glm::sin(rTime*(ii*0.37f) + ii * 0.67f) * (mDims * 0.5f - 8.0f);
                    position.z = glm::cos(rTime*(ii*0.11f) + ii * 0.13f) * (mDims * 0.5f - 8.0f);
                    radius = (2.0f + (glm::sin(rTime*(ii*0.13f))*0.5f + 0.5f)*2.0f);

                    spatial->setPosition(position);
                    spatial->setScale(glm::vec3(radius));
                }
                MICROPROFILE_LEAVE();
            }

            if (!mDirtyBalls) {
                return;
            }

			// Stats.
			uint32_t numVertices = 0;
			uint32_t maxVertices = (32<<10);

            const uint32_t ypitch = mDims;
		    const uint32_t zpitch = mDims*mDims;
		    const float invdim = 1.0f/float(mDims-1);

            MICROPROFILE_ENTERI("Metaballs System", "generateGrid", MP_AUTO);
            Grid* grid = new Grid[mDims * mDims * mDims];
			for (uint32_t zz = 0; zz < mDims; ++zz) {
				for (uint32_t yy = 0; yy < mDims; ++yy) {
					uint32_t offset = (zz*mDims+yy)*mDims;

					for (uint32_t xx = 0; xx < mDims; ++xx) {
						uint32_t xoffset = offset + xx;

						float dist = 0.0f;
						float prod = 1.0f;
						for (uint32_t ii = 0; ii < balls.size(); ++ii) {
                            auto spatial = balls[ii]->get<SpatialComponent>();
							float dx = spatial->getPosition().x - (-mDims*0.5f + float(xx) );
							float dy = spatial->getPosition().y - (-mDims*0.5f + float(yy) );
							float dz = spatial->getPosition().z - (-mDims*0.5f + float(zz) );

                            float invr = 1.f / spatial->getScale().x;
							float dot = dx*dx + dy*dy + dz*dz;
							dot *= invr*invr;

							dist *= dot;
							dist += prod;
							prod *= dot;
						}

						grid[xoffset].mVal = dist / prod - 1.0f;
					}
				}
			}

			for (uint32_t zz = 1; zz < mDims-1; ++zz) {
				for (uint32_t yy = 1; yy < mDims-1; ++yy) {
					uint32_t offset = (zz*mDims+yy)*mDims;

					for (uint32_t xx = 1; xx < mDims-1; ++xx) {
						uint32_t xoffset = offset + xx;

						// Grid* grid = grid;
                        grid[xoffset].mNormal = glm::normalize(glm::vec3(
							grid[xoffset-1     ].mVal - grid[xoffset+1     ].mVal,
							grid[xoffset-ypitch].mVal - grid[xoffset+ypitch].mVal,
							grid[xoffset-zpitch].mVal - grid[xoffset+zpitch].mVal
						));
					}
				}
			}
            MICROPROFILE_LEAVE();

            MICROPROFILE_ENTERI("Metaballs System", "generateMesh", MP_AUTO);
            std::vector<float> vertices;
            std::vector<float> normals;
            for (uint32_t zz = 0; zz < mDims - 1 && numVertices + 12 < maxVertices; ++zz) {
                float rgb[6];
                rgb[2] = zz * invdim;
                rgb[5] = (zz + 1)*invdim;

                for (uint32_t yy = 0; yy < mDims - 1 && numVertices + 12 < maxVertices; ++yy) {
                    uint32_t offset = (zz*mDims + yy)*mDims;

                    rgb[1] = yy * invdim;
                    rgb[4] = (yy + 1)*invdim;

                    for (uint32_t xx = 0; xx < mDims - 1 && numVertices + 12 < maxVertices; ++xx) {
                        uint32_t xoffset = offset + xx;

                        rgb[0] = xx * invdim;
                        rgb[3] = (xx + 1)*invdim;

                        float pos[3] =
                        {
                            -mDims * 0.5f + float(xx),
                            -mDims * 0.5f + float(yy),
                            -mDims * 0.5f + float(zz)
                        };

                        // const Grid* grid = mGrid;
                        const Grid* val[8] = {
                            &grid[xoffset + zpitch + ypitch],
                            &grid[xoffset + zpitch + ypitch + 1],
                            &grid[xoffset + ypitch + 1],
                            &grid[xoffset + ypitch],
                            &grid[xoffset + zpitch],
                            &grid[xoffset + zpitch + 1],
                            &grid[xoffset + 1],
                            &grid[xoffset],
                        };

                        uint32_t num = triangulate(vertices, normals, rgb, pos, val, 0.5f);
                        numVertices += num;
                    }
                }
            }
            MICROPROFILE_LEAVE();

            MICROPROFILE_ENTERI("Metaballs System", "updateMesh", MP_AUTO);
            metaballMesh->mMesh->updateVertexBuffer(VertexType::Position, vertices);
            metaballMesh->mMesh->updateVertexBuffer(VertexType::Normal, normals);
            mDirtyBalls = false;
            delete[] grid;
            MICROPROFILE_LEAVE();
        }

    private:

        uint32_t triangulate(
              std::vector<float>& vertices
            , std::vector<float>& normals
            , const float* _rgb
            , const float* _xyz
            , const Grid* _val[8]
            , float _iso
        )
        {
            uint8_t cubeindex = 0;
            cubeindex |= (_val[0]->mVal < _iso) ? 0x01 : 0;
            cubeindex |= (_val[1]->mVal < _iso) ? 0x02 : 0;
            cubeindex |= (_val[2]->mVal < _iso) ? 0x04 : 0;
            cubeindex |= (_val[3]->mVal < _iso) ? 0x08 : 0;
            cubeindex |= (_val[4]->mVal < _iso) ? 0x10 : 0;
            cubeindex |= (_val[5]->mVal < _iso) ? 0x20 : 0;
            cubeindex |= (_val[6]->mVal < _iso) ? 0x40 : 0;
            cubeindex |= (_val[7]->mVal < _iso) ? 0x80 : 0;

            if (0 == sEdges[cubeindex])
            {
                return 0;
            }

            float verts[12][6];
            uint16_t flags = sEdges[cubeindex];

            for (uint32_t ii = 0; ii < 12; ++ii)
            {
                if (flags & (1 << ii))
                {
                    uint32_t idx0 = ii & 7;
                    uint32_t idx1 = "\x1\x2\x3\x0\x5\x6\x7\x4\x4\x5\x6\x7"[ii];
                    float* vertex = verts[ii];
                    float lerp = vertLerp(vertex, _iso, idx0, _val[idx0]->mVal, idx1, _val[idx1]->mVal);

                    const float* na = &_val[idx0]->mNormal[0];
                    const float* nb = &_val[idx1]->mNormal[0];
                    vertex[3] = na[0] + lerp * (nb[0] - na[0]);
                    vertex[4] = na[1] + lerp * (nb[1] - na[1]);
                    vertex[5] = na[2] + lerp * (nb[2] - na[2]);
                }
            }

            float dr = _rgb[3] - _rgb[0];
            float dg = _rgb[4] - _rgb[1];
            float db = _rgb[5] - _rgb[2];

            uint32_t num = 0;
            const int8_t* indices = sIndices[cubeindex];
            for (uint32_t ii = 0; indices[ii] != -1; ++ii)
            {
                const float* _vertex = verts[uint8_t(indices[ii])];

                vertices.push_back(_xyz[0] + _vertex[0]);
                vertices.push_back(_xyz[1] + _vertex[1]);
                vertices.push_back(_xyz[2] + _vertex[2]);

                normals.push_back(_vertex[3]);
                normals.push_back(_vertex[4]);
                normals.push_back(_vertex[5]);
                
                ++num;
            }

            return num;
        }

        float vertLerp(float* _result, float _iso, uint32_t _idx0, float _v0, uint32_t _idx1, float _v1) {
            const float* edge0 = sCube[_idx0];
            const float* edge1 = sCube[_idx1];

            if (glm::abs(_iso - _v1) < 0.00001f) {
                _result[0] = edge1[0];
                _result[1] = edge1[1];
                _result[2] = edge1[2];
                return 1.0f;
            }

            if (glm::abs(_iso - _v0) < 0.00001f || glm::abs(_v0 - _v1) < 0.00001f) {
                _result[0] = edge0[0];
                _result[1] = edge0[1];
                _result[2] = edge0[2];
                return 0.0f;
            }

            float lerp = (_iso - _v0) / (_v1 - _v0);
            _result[0] = edge0[0] + lerp * (edge1[0] - edge0[0]);
            _result[1] = edge0[1] + lerp * (edge1[1] - edge0[1]);
            _result[2] = edge0[2] + lerp * (edge1[2] - edge0[2]);

            return lerp;
        }

        const uint16_t sEdges[256] = {
            0x000, 0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c,
            0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09, 0xf00,
            0x190, 0x099, 0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c,
            0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90,
            0x230, 0x339, 0x033, 0x13a, 0x636, 0x73f, 0x435, 0x53c,
            0xa3c, 0xb35, 0x83f, 0x936, 0xe3a, 0xf33, 0xc39, 0xd30,
            0x3a0, 0x2a9, 0x1a3, 0x0aa, 0x7a6, 0x6af, 0x5a5, 0x4ac,
            0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0,
            0x460, 0x569, 0x663, 0x76a, 0x66 , 0x16f, 0x265, 0x36c,
            0xc6c, 0xd65, 0xe6f, 0xf66, 0x86a, 0x963, 0xa69, 0xb60,
            0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0x0ff, 0x3f5, 0x2fc,
            0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0,
            0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x055, 0x15c,
            0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950,
            0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0x0cc,
            0xfcc, 0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0,
            0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf, 0xec5, 0xfcc,
            0x0cc, 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0,
            0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c,
            0x15c, 0x55 , 0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650,
            0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6, 0xfff, 0xcf5, 0xdfc,
            0x2fc, 0x3f5, 0x0ff, 0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0,
            0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c,
            0x36c, 0x265, 0x16f, 0x066, 0x76a, 0x663, 0x569, 0x460,
            0xca0, 0xda9, 0xea3, 0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac,
            0x4ac, 0x5a5, 0x6af, 0x7a6, 0x0aa, 0x1a3, 0x2a9, 0x3a0,
            0xd30, 0xc39, 0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c,
            0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x033, 0x339, 0x230,
            0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c,
            0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x099, 0x190,
            0xf00, 0xe09, 0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c,
            0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203, 0x109, 0x000,
        };

        const int8_t sIndices[256][16] = {
        	{  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   0,  8,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   0,  1,  9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   1,  8,  3,  9,  8,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   1,  2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   0,  8,  3,  1,  2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   9,  2, 10,  0,  2,  9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   2,  8,  3,  2, 10,  8, 10,  9,  8, -1, -1, -1, -1, -1, -1, -1 },
        	{   3, 11,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   0, 11,  2,  8, 11,  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   1,  9,  0,  2,  3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   1, 11,  2,  1,  9, 11,  9,  8, 11, -1, -1, -1, -1, -1, -1, -1 },
        	{   3, 10,  1, 11, 10,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   0, 10,  1,  0,  8, 10,  8, 11, 10, -1, -1, -1, -1, -1, -1, -1 },
        	{   3,  9,  0,  3, 11,  9, 11, 10,  9, -1, -1, -1, -1, -1, -1, -1 },
        	{   9,  8, 10, 10,  8, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   4,  7,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   4,  3,  0,  7,  3,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   0,  1,  9,  8,  4,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   4,  1,  9,  4,  7,  1,  7,  3,  1, -1, -1, -1, -1, -1, -1, -1 },
        	{   1,  2, 10,  8,  4,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   3,  4,  7,  3,  0,  4,  1,  2, 10, -1, -1, -1, -1, -1, -1, -1 },
        	{   9,  2, 10,  9,  0,  2,  8,  4,  7, -1, -1, -1, -1, -1, -1, -1 },
        	{   2, 10,  9,  2,  9,  7,  2,  7,  3,  7,  9,  4, -1, -1, -1, -1 },
        	{   8,  4,  7,  3, 11,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{  11,  4,  7, 11,  2,  4,  2,  0,  4, -1, -1, -1, -1, -1, -1, -1 },
        	{   9,  0,  1,  8,  4,  7,  2,  3, 11, -1, -1, -1, -1, -1, -1, -1 },
        	{   4,  7, 11,  9,  4, 11,  9, 11,  2,  9,  2,  1, -1, -1, -1, -1 },
        	{   3, 10,  1,  3, 11, 10,  7,  8,  4, -1, -1, -1, -1, -1, -1, -1 },
        	{   1, 11, 10,  1,  4, 11,  1,  0,  4,  7, 11,  4, -1, -1, -1, -1 },
        	{   4,  7,  8,  9,  0, 11,  9, 11, 10, 11,  0,  3, -1, -1, -1, -1 },
        	{   4,  7, 11,  4, 11,  9,  9, 11, 10, -1, -1, -1, -1, -1, -1, -1 },
        	{   9,  5,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   9,  5,  4,  0,  8,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   0,  5,  4,  1,  5,  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   8,  5,  4,  8,  3,  5,  3,  1,  5, -1, -1, -1, -1, -1, -1, -1 },
        	{   1,  2, 10,  9,  5,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   3,  0,  8,  1,  2, 10,  4,  9,  5, -1, -1, -1, -1, -1, -1, -1 },
        	{   5,  2, 10,  5,  4,  2,  4,  0,  2, -1, -1, -1, -1, -1, -1, -1 },
        	{   2, 10,  5,  3,  2,  5,  3,  5,  4,  3,  4,  8, -1, -1, -1, -1 },
        	{   9,  5,  4,  2,  3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   0, 11,  2,  0,  8, 11,  4,  9,  5, -1, -1, -1, -1, -1, -1, -1 },
        	{   0,  5,  4,  0,  1,  5,  2,  3, 11, -1, -1, -1, -1, -1, -1, -1 },
        	{   2,  1,  5,  2,  5,  8,  2,  8, 11,  4,  8,  5, -1, -1, -1, -1 },
        	{  10,  3, 11, 10,  1,  3,  9,  5,  4, -1, -1, -1, -1, -1, -1, -1 },
        	{   4,  9,  5,  0,  8,  1,  8, 10,  1,  8, 11, 10, -1, -1, -1, -1 },
        	{   5,  4,  0,  5,  0, 11,  5, 11, 10, 11,  0,  3, -1, -1, -1, -1 },
        	{   5,  4,  8,  5,  8, 10, 10,  8, 11, -1, -1, -1, -1, -1, -1, -1 },
        	{   9,  7,  8,  5,  7,  9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   9,  3,  0,  9,  5,  3,  5,  7,  3, -1, -1, -1, -1, -1, -1, -1 },
        	{   0,  7,  8,  0,  1,  7,  1,  5,  7, -1, -1, -1, -1, -1, -1, -1 },
        	{   1,  5,  3,  3,  5,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   9,  7,  8,  9,  5,  7, 10,  1,  2, -1, -1, -1, -1, -1, -1, -1 },
        	{  10,  1,  2,  9,  5,  0,  5,  3,  0,  5,  7,  3, -1, -1, -1, -1 },
        	{   8,  0,  2,  8,  2,  5,  8,  5,  7, 10,  5,  2, -1, -1, -1, -1 },
        	{   2, 10,  5,  2,  5,  3,  3,  5,  7, -1, -1, -1, -1, -1, -1, -1 },
        	{   7,  9,  5,  7,  8,  9,  3, 11,  2, -1, -1, -1, -1, -1, -1, -1 },
        	{   9,  5,  7,  9,  7,  2,  9,  2,  0,  2,  7, 11, -1, -1, -1, -1 },
        	{   2,  3, 11,  0,  1,  8,  1,  7,  8,  1,  5,  7, -1, -1, -1, -1 },
        	{  11,  2,  1, 11,  1,  7,  7,  1,  5, -1, -1, -1, -1, -1, -1, -1 },
        	{   9,  5,  8,  8,  5,  7, 10,  1,  3, 10,  3, 11, -1, -1, -1, -1 },
        	{   5,  7,  0,  5,  0,  9,  7, 11,  0,  1,  0, 10, 11, 10,  0, -1 },
        	{  11, 10,  0, 11,  0,  3, 10,  5,  0,  8,  0,  7,  5,  7,  0, -1 },
        	{  11, 10,  5,  7, 11,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{  10,  6,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   0,  8,  3,  5, 10,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   9,  0,  1,  5, 10,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   1,  8,  3,  1,  9,  8,  5, 10,  6, -1, -1, -1, -1, -1, -1, -1 },
        	{   1,  6,  5,  2,  6,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   1,  6,  5,  1,  2,  6,  3,  0,  8, -1, -1, -1, -1, -1, -1, -1 },
        	{   9,  6,  5,  9,  0,  6,  0,  2,  6, -1, -1, -1, -1, -1, -1, -1 },
        	{   5,  9,  8,  5,  8,  2,  5,  2,  6,  3,  2,  8, -1, -1, -1, -1 },
        	{   2,  3, 11, 10,  6,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{  11,  0,  8, 11,  2,  0, 10,  6,  5, -1, -1, -1, -1, -1, -1, -1 },
        	{   0,  1,  9,  2,  3, 11,  5, 10,  6, -1, -1, -1, -1, -1, -1, -1 },
        	{   5, 10,  6,  1,  9,  2,  9, 11,  2,  9,  8, 11, -1, -1, -1, -1 },
        	{   6,  3, 11,  6,  5,  3,  5,  1,  3, -1, -1, -1, -1, -1, -1, -1 },
        	{   0,  8, 11,  0, 11,  5,  0,  5,  1,  5, 11,  6, -1, -1, -1, -1 },
        	{   3, 11,  6,  0,  3,  6,  0,  6,  5,  0,  5,  9, -1, -1, -1, -1 },
        	{   6,  5,  9,  6,  9, 11, 11,  9,  8, -1, -1, -1, -1, -1, -1, -1 },
        	{   5, 10,  6,  4,  7,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   4,  3,  0,  4,  7,  3,  6,  5, 10, -1, -1, -1, -1, -1, -1, -1 },
        	{   1,  9,  0,  5, 10,  6,  8,  4,  7, -1, -1, -1, -1, -1, -1, -1 },
        	{  10,  6,  5,  1,  9,  7,  1,  7,  3,  7,  9,  4, -1, -1, -1, -1 },
        	{   6,  1,  2,  6,  5,  1,  4,  7,  8, -1, -1, -1, -1, -1, -1, -1 },
        	{   1,  2,  5,  5,  2,  6,  3,  0,  4,  3,  4,  7, -1, -1, -1, -1 },
        	{   8,  4,  7,  9,  0,  5,  0,  6,  5,  0,  2,  6, -1, -1, -1, -1 },
        	{   7,  3,  9,  7,  9,  4,  3,  2,  9,  5,  9,  6,  2,  6,  9, -1 },
        	{   3, 11,  2,  7,  8,  4, 10,  6,  5, -1, -1, -1, -1, -1, -1, -1 },
        	{   5, 10,  6,  4,  7,  2,  4,  2,  0,  2,  7, 11, -1, -1, -1, -1 },
        	{   0,  1,  9,  4,  7,  8,  2,  3, 11,  5, 10,  6, -1, -1, -1, -1 },
        	{   9,  2,  1,  9, 11,  2,  9,  4, 11,  7, 11,  4,  5, 10,  6, -1 },
        	{   8,  4,  7,  3, 11,  5,  3,  5,  1,  5, 11,  6, -1, -1, -1, -1 },
        	{   5,  1, 11,  5, 11,  6,  1,  0, 11,  7, 11,  4,  0,  4, 11, -1 },
        	{   0,  5,  9,  0,  6,  5,  0,  3,  6, 11,  6,  3,  8,  4,  7, -1 },
        	{   6,  5,  9,  6,  9, 11,  4,  7,  9,  7, 11,  9, -1, -1, -1, -1 },
        	{  10,  4,  9,  6,  4, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   4, 10,  6,  4,  9, 10,  0,  8,  3, -1, -1, -1, -1, -1, -1, -1 },
        	{  10,  0,  1, 10,  6,  0,  6,  4,  0, -1, -1, -1, -1, -1, -1, -1 },
        	{   8,  3,  1,  8,  1,  6,  8,  6,  4,  6,  1, 10, -1, -1, -1, -1 },
        	{   1,  4,  9,  1,  2,  4,  2,  6,  4, -1, -1, -1, -1, -1, -1, -1 },
        	{   3,  0,  8,  1,  2,  9,  2,  4,  9,  2,  6,  4, -1, -1, -1, -1 },
        	{   0,  2,  4,  4,  2,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   8,  3,  2,  8,  2,  4,  4,  2,  6, -1, -1, -1, -1, -1, -1, -1 },
        	{  10,  4,  9, 10,  6,  4, 11,  2,  3, -1, -1, -1, -1, -1, -1, -1 },
        	{   0,  8,  2,  2,  8, 11,  4,  9, 10,  4, 10,  6, -1, -1, -1, -1 },
        	{   3, 11,  2,  0,  1,  6,  0,  6,  4,  6,  1, 10, -1, -1, -1, -1 },
        	{   6,  4,  1,  6,  1, 10,  4,  8,  1,  2,  1, 11,  8, 11,  1, -1 },
        	{   9,  6,  4,  9,  3,  6,  9,  1,  3, 11,  6,  3, -1, -1, -1, -1 },
        	{   8, 11,  1,  8,  1,  0, 11,  6,  1,  9,  1,  4,  6,  4,  1, -1 },
        	{   3, 11,  6,  3,  6,  0,  0,  6,  4, -1, -1, -1, -1, -1, -1, -1 },
        	{   6,  4,  8, 11,  6,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   7, 10,  6,  7,  8, 10,  8,  9, 10, -1, -1, -1, -1, -1, -1, -1 },
        	{   0,  7,  3,  0, 10,  7,  0,  9, 10,  6,  7, 10, -1, -1, -1, -1 },
        	{  10,  6,  7,  1, 10,  7,  1,  7,  8,  1,  8,  0, -1, -1, -1, -1 },
        	{  10,  6,  7, 10,  7,  1,  1,  7,  3, -1, -1, -1, -1, -1, -1, -1 },
        	{   1,  2,  6,  1,  6,  8,  1,  8,  9,  8,  6,  7, -1, -1, -1, -1 },
        	{   2,  6,  9,  2,  9,  1,  6,  7,  9,  0,  9,  3,  7,  3,  9, -1 },
        	{   7,  8,  0,  7,  0,  6,  6,  0,  2, -1, -1, -1, -1, -1, -1, -1 },
        	{   7,  3,  2,  6,  7,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   2,  3, 11, 10,  6,  8, 10,  8,  9,  8,  6,  7, -1, -1, -1, -1 },
        	{   2,  0,  7,  2,  7, 11,  0,  9,  7,  6,  7, 10,  9, 10,  7, -1 },
        	{   1,  8,  0,  1,  7,  8,  1, 10,  7,  6,  7, 10,  2,  3, 11, -1 },
        	{  11,  2,  1, 11,  1,  7, 10,  6,  1,  6,  7,  1, -1, -1, -1, -1 },
        	{   8,  9,  6,  8,  6,  7,  9,  1,  6, 11,  6,  3,  1,  3,  6, -1 },
        	{   0,  9,  1, 11,  6,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   7,  8,  0,  7,  0,  6,  3, 11,  0, 11,  6,  0, -1, -1, -1, -1 },
        	{   7, 11,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   7,  6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   3,  0,  8, 11,  7,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   0,  1,  9, 11,  7,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   8,  1,  9,  8,  3,  1, 11,  7,  6, -1, -1, -1, -1, -1, -1, -1 },
        	{  10,  1,  2,  6, 11,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   1,  2, 10,  3,  0,  8,  6, 11,  7, -1, -1, -1, -1, -1, -1, -1 },
        	{   2,  9,  0,  2, 10,  9,  6, 11,  7, -1, -1, -1, -1, -1, -1, -1 },
        	{   6, 11,  7,  2, 10,  3, 10,  8,  3, 10,  9,  8, -1, -1, -1, -1 },
        	{   7,  2,  3,  6,  2,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   7,  0,  8,  7,  6,  0,  6,  2,  0, -1, -1, -1, -1, -1, -1, -1 },
        	{   2,  7,  6,  2,  3,  7,  0,  1,  9, -1, -1, -1, -1, -1, -1, -1 },
        	{   1,  6,  2,  1,  8,  6,  1,  9,  8,  8,  7,  6, -1, -1, -1, -1 },
        	{  10,  7,  6, 10,  1,  7,  1,  3,  7, -1, -1, -1, -1, -1, -1, -1 },
        	{  10,  7,  6,  1,  7, 10,  1,  8,  7,  1,  0,  8, -1, -1, -1, -1 },
        	{   0,  3,  7,  0,  7, 10,  0, 10,  9,  6, 10,  7, -1, -1, -1, -1 },
        	{   7,  6, 10,  7, 10,  8,  8, 10,  9, -1, -1, -1, -1, -1, -1, -1 },
        	{   6,  8,  4, 11,  8,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   3,  6, 11,  3,  0,  6,  0,  4,  6, -1, -1, -1, -1, -1, -1, -1 },
        	{   8,  6, 11,  8,  4,  6,  9,  0,  1, -1, -1, -1, -1, -1, -1, -1 },
        	{   9,  4,  6,  9,  6,  3,  9,  3,  1, 11,  3,  6, -1, -1, -1, -1 },
        	{   6,  8,  4,  6, 11,  8,  2, 10,  1, -1, -1, -1, -1, -1, -1, -1 },
        	{   1,  2, 10,  3,  0, 11,  0,  6, 11,  0,  4,  6, -1, -1, -1, -1 },
        	{   4, 11,  8,  4,  6, 11,  0,  2,  9,  2, 10,  9, -1, -1, -1, -1 },
        	{  10,  9,  3, 10,  3,  2,  9,  4,  3, 11,  3,  6,  4,  6,  3, -1 },
        	{   8,  2,  3,  8,  4,  2,  4,  6,  2, -1, -1, -1, -1, -1, -1, -1 },
        	{   0,  4,  2,  4,  6,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   1,  9,  0,  2,  3,  4,  2,  4,  6,  4,  3,  8, -1, -1, -1, -1 },
        	{   1,  9,  4,  1,  4,  2,  2,  4,  6, -1, -1, -1, -1, -1, -1, -1 },
        	{   8,  1,  3,  8,  6,  1,  8,  4,  6,  6, 10,  1, -1, -1, -1, -1 },
        	{  10,  1,  0, 10,  0,  6,  6,  0,  4, -1, -1, -1, -1, -1, -1, -1 },
        	{   4,  6,  3,  4,  3,  8,  6, 10,  3,  0,  3,  9, 10,  9,  3, -1 },
        	{  10,  9,  4,  6, 10,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   4,  9,  5,  7,  6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   0,  8,  3,  4,  9,  5, 11,  7,  6, -1, -1, -1, -1, -1, -1, -1 },
        	{   5,  0,  1,  5,  4,  0,  7,  6, 11, -1, -1, -1, -1, -1, -1, -1 },
        	{  11,  7,  6,  8,  3,  4,  3,  5,  4,  3,  1,  5, -1, -1, -1, -1 },
        	{   9,  5,  4, 10,  1,  2,  7,  6, 11, -1, -1, -1, -1, -1, -1, -1 },
        	{   6, 11,  7,  1,  2, 10,  0,  8,  3,  4,  9,  5, -1, -1, -1, -1 },
        	{   7,  6, 11,  5,  4, 10,  4,  2, 10,  4,  0,  2, -1, -1, -1, -1 },
        	{   3,  4,  8,  3,  5,  4,  3,  2,  5, 10,  5,  2, 11,  7,  6, -1 },
        	{   7,  2,  3,  7,  6,  2,  5,  4,  9, -1, -1, -1, -1, -1, -1, -1 },
        	{   9,  5,  4,  0,  8,  6,  0,  6,  2,  6,  8,  7, -1, -1, -1, -1 },
        	{   3,  6,  2,  3,  7,  6,  1,  5,  0,  5,  4,  0, -1, -1, -1, -1 },
        	{   6,  2,  8,  6,  8,  7,  2,  1,  8,  4,  8,  5,  1,  5,  8, -1 },
        	{   9,  5,  4, 10,  1,  6,  1,  7,  6,  1,  3,  7, -1, -1, -1, -1 },
        	{   1,  6, 10,  1,  7,  6,  1,  0,  7,  8,  7,  0,  9,  5,  4, -1 },
        	{   4,  0, 10,  4, 10,  5,  0,  3, 10,  6, 10,  7,  3,  7, 10, -1 },
        	{   7,  6, 10,  7, 10,  8,  5,  4, 10,  4,  8, 10, -1, -1, -1, -1 },
        	{   6,  9,  5,  6, 11,  9, 11,  8,  9, -1, -1, -1, -1, -1, -1, -1 },
        	{   3,  6, 11,  0,  6,  3,  0,  5,  6,  0,  9,  5, -1, -1, -1, -1 },
        	{   0, 11,  8,  0,  5, 11,  0,  1,  5,  5,  6, 11, -1, -1, -1, -1 },
        	{   6, 11,  3,  6,  3,  5,  5,  3,  1, -1, -1, -1, -1, -1, -1, -1 },
        	{   1,  2, 10,  9,  5, 11,  9, 11,  8, 11,  5,  6, -1, -1, -1, -1 },
        	{   0, 11,  3,  0,  6, 11,  0,  9,  6,  5,  6,  9,  1,  2, 10, -1 },
        	{  11,  8,  5, 11,  5,  6,  8,  0,  5, 10,  5,  2,  0,  2,  5, -1 },
        	{   6, 11,  3,  6,  3,  5,  2, 10,  3, 10,  5,  3, -1, -1, -1, -1 },
        	{   5,  8,  9,  5,  2,  8,  5,  6,  2,  3,  8,  2, -1, -1, -1, -1 },
        	{   9,  5,  6,  9,  6,  0,  0,  6,  2, -1, -1, -1, -1, -1, -1, -1 },
        	{   1,  5,  8,  1,  8,  0,  5,  6,  8,  3,  8,  2,  6,  2,  8, -1 },
        	{   1,  5,  6,  2,  1,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   1,  3,  6,  1,  6, 10,  3,  8,  6,  5,  6,  9,  8,  9,  6, -1 },
        	{  10,  1,  0, 10,  0,  6,  9,  5,  0,  5,  6,  0, -1, -1, -1, -1 },
        	{   0,  3,  8,  5,  6, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{  10,  5,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{  11,  5, 10,  7,  5, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{  11,  5, 10, 11,  7,  5,  8,  3,  0, -1, -1, -1, -1, -1, -1, -1 },
        	{   5, 11,  7,  5, 10, 11,  1,  9,  0, -1, -1, -1, -1, -1, -1, -1 },
        	{  10,  7,  5, 10, 11,  7,  9,  8,  1,  8,  3,  1, -1, -1, -1, -1 },
        	{  11,  1,  2, 11,  7,  1,  7,  5,  1, -1, -1, -1, -1, -1, -1, -1 },
        	{   0,  8,  3,  1,  2,  7,  1,  7,  5,  7,  2, 11, -1, -1, -1, -1 },
        	{   9,  7,  5,  9,  2,  7,  9,  0,  2,  2, 11,  7, -1, -1, -1, -1 },
        	{   7,  5,  2,  7,  2, 11,  5,  9,  2,  3,  2,  8,  9,  8,  2, -1 },
        	{   2,  5, 10,  2,  3,  5,  3,  7,  5, -1, -1, -1, -1, -1, -1, -1 },
        	{   8,  2,  0,  8,  5,  2,  8,  7,  5, 10,  2,  5, -1, -1, -1, -1 },
        	{   9,  0,  1,  5, 10,  3,  5,  3,  7,  3, 10,  2, -1, -1, -1, -1 },
        	{   9,  8,  2,  9,  2,  1,  8,  7,  2, 10,  2,  5,  7,  5,  2, -1 },
        	{   1,  3,  5,  3,  7,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   0,  8,  7,  0,  7,  1,  1,  7,  5, -1, -1, -1, -1, -1, -1, -1 },
        	{   9,  0,  3,  9,  3,  5,  5,  3,  7, -1, -1, -1, -1, -1, -1, -1 },
        	{   9,  8,  7,  5,  9,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   5,  8,  4,  5, 10,  8, 10, 11,  8, -1, -1, -1, -1, -1, -1, -1 },
        	{   5,  0,  4,  5, 11,  0,  5, 10, 11, 11,  3,  0, -1, -1, -1, -1 },
        	{   0,  1,  9,  8,  4, 10,  8, 10, 11, 10,  4,  5, -1, -1, -1, -1 },
        	{  10, 11,  4, 10,  4,  5, 11,  3,  4,  9,  4,  1,  3,  1,  4, -1 },
        	{   2,  5,  1,  2,  8,  5,  2, 11,  8,  4,  5,  8, -1, -1, -1, -1 },
        	{   0,  4, 11,  0, 11,  3,  4,  5, 11,  2, 11,  1,  5,  1, 11, -1 },
        	{   0,  2,  5,  0,  5,  9,  2, 11,  5,  4,  5,  8, 11,  8,  5, -1 },
        	{   9,  4,  5,  2, 11,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   2,  5, 10,  3,  5,  2,  3,  4,  5,  3,  8,  4, -1, -1, -1, -1 },
        	{   5, 10,  2,  5,  2,  4,  4,  2,  0, -1, -1, -1, -1, -1, -1, -1 },
        	{   3, 10,  2,  3,  5, 10,  3,  8,  5,  4,  5,  8,  0,  1,  9, -1 },
        	{   5, 10,  2,  5,  2,  4,  1,  9,  2,  9,  4,  2, -1, -1, -1, -1 },
        	{   8,  4,  5,  8,  5,  3,  3,  5,  1, -1, -1, -1, -1, -1, -1, -1 },
        	{   0,  4,  5,  1,  0,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   8,  4,  5,  8,  5,  3,  9,  0,  5,  0,  3,  5, -1, -1, -1, -1 },
        	{   9,  4,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   4, 11,  7,  4,  9, 11,  9, 10, 11, -1, -1, -1, -1, -1, -1, -1 },
        	{   0,  8,  3,  4,  9,  7,  9, 11,  7,  9, 10, 11, -1, -1, -1, -1 },
        	{   1, 10, 11,  1, 11,  4,  1,  4,  0,  7,  4, 11, -1, -1, -1, -1 },
        	{   3,  1,  4,  3,  4,  8,  1, 10,  4,  7,  4, 11, 10, 11,  4, -1 },
        	{   4, 11,  7,  9, 11,  4,  9,  2, 11,  9,  1,  2, -1, -1, -1, -1 },
        	{   9,  7,  4,  9, 11,  7,  9,  1, 11,  2, 11,  1,  0,  8,  3, -1 },
        	{  11,  7,  4, 11,  4,  2,  2,  4,  0, -1, -1, -1, -1, -1, -1, -1 },
        	{  11,  7,  4, 11,  4,  2,  8,  3,  4,  3,  2,  4, -1, -1, -1, -1 },
        	{   2,  9, 10,  2,  7,  9,  2,  3,  7,  7,  4,  9, -1, -1, -1, -1 },
        	{   9, 10,  7,  9,  7,  4, 10,  2,  7,  8,  7,  0,  2,  0,  7, -1 },
        	{   3,  7, 10,  3, 10,  2,  7,  4, 10,  1, 10,  0,  4,  0, 10, -1 },
        	{   1, 10,  2,  8,  7,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   4,  9,  1,  4,  1,  7,  7,  1,  3, -1, -1, -1, -1, -1, -1, -1 },
        	{   4,  9,  1,  4,  1,  7,  0,  8,  1,  8,  7,  1, -1, -1, -1, -1 },
        	{   4,  0,  3,  7,  4,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   4,  8,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   9, 10,  8, 10, 11,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   3,  0,  9,  3,  9, 11, 11,  9, 10, -1, -1, -1, -1, -1, -1, -1 },
        	{   0,  1, 10,  0, 10,  8,  8, 10, 11, -1, -1, -1, -1, -1, -1, -1 },
        	{   3,  1, 10, 11,  3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   1,  2, 11,  1, 11,  9,  9, 11,  8, -1, -1, -1, -1, -1, -1, -1 },
        	{   3,  0,  9,  3,  9, 11,  1,  2,  9,  2, 11,  9, -1, -1, -1, -1 },
        	{   0,  2, 11,  8,  0, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   3,  2, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   2,  3,  8,  2,  8, 10, 10,  8,  9, -1, -1, -1, -1, -1, -1, -1 },
        	{   9, 10,  2,  0,  9,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   2,  3,  8,  2,  8, 10,  0,  1,  8,  1, 10,  8, -1, -1, -1, -1 },
        	{   1, 10,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   1,  3,  8,  9,  1,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   0,  9,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{   0,  3,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        	{  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        };

        const float sCube[8][3] = {
        	{ 0.0f, 1.0f, 1.0f }, // 0
        	{ 1.0f, 1.0f, 1.0f }, // 1
        	{ 1.0f, 1.0f, 0.0f }, // 2
        	{ 0.0f, 1.0f, 0.0f }, // 3
        	{ 0.0f, 0.0f, 1.0f }, // 4
        	{ 1.0f, 0.0f, 1.0f }, // 5
        	{ 1.0f, 0.0f, 0.0f }, // 6
        	{ 0.0f, 0.0f, 0.0f }, // 7
        };
};
