#pragma once

#include "ECS/Component/Component.hpp"
#include "Renderer/GLObjects/Texture.hpp"

#include <glm/glm.hpp>
#include <imgui/imgui.h>

using namespace neo;

namespace Froxels {
    struct VolumeComponent : public Component {
        VolumeComponent(Texture* tex)
            : mTexture(tex)
        {}
        glm::ivec3 getVoxelIndex(size_t index) const {
            int line = 0x1 << mSize;
            int slice = (0x1 << mSize) * line;
            int z =  static_cast<int>(index) / slice;
            int y = (static_cast<int>(index) - z * slice) / line;
            int x = (static_cast<int>(index) - z * slice - y * line);
            return glm::ivec3(x, y, z);
        }

        virtual std::string getName() const override { return "VolumeComponent"; }
        virtual void imGuiEditor() override {
            int numVoxels = static_cast<int>(std::pow(0x1 << mSize, 3));
            ImGui::Text("Dimension : %d*%d*%d=%d", 0x1 << mSize, 0x1 << mSize, 0x1 << mSize, numVoxels);
            bool resize = false;
            bool upload = false;
            resize |= ImGui::SliderInt("Size", &mSize, 1, 7);

            upload |= ImGui::Button("Reload Volume");

            static int mode = 0;
            ImGui::RadioButton("Clear", &mode, 0);
            ImGui::RadioButton("Positions", &mode, 1);
            ImGui::RadioButton("Randomize", &mode, 2);
            ImGui::RadioButton("Randomize w/ alpha", &mode, 3);
            if (resize) {
                mTexture->resize(glm::uvec3(0x1 << mSize));
            }
            if (upload) {
                std::vector<glm::vec4> data;
                mTexture->resize(glm::uvec3(numVoxels));
                data.resize(numVoxels);
                for (int i = 0; i < numVoxels; i++) {

                    if (mode == 0) {
                        data[i] = glm::vec4(0);
                    }
                    else if (mode == 1) {
                        glm::vec3 pos = glm::vec3(getVoxelIndex(i)) / glm::vec3(static_cast<float>(0x1<<mSize));
                        data[i] = glm::vec4(pos.x, pos.y, pos.z, 1.0);
                    }
                    else if (mode == 2) {
                        data[i] = glm::vec4(util::genRandomVec3(), util::genRandom());
                    }
                    else if (mode == 2) {
                        data[i] = glm::vec4(util::genRandomVec3(), 1.0);
                    }
                }
                mTexture->update(glm::uvec3(0x1<<mSize), data.data());
            }
        }
        int mSize = 3;
        Texture* mTexture;
    };
}
