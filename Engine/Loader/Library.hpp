#pragma once

#include "Renderer/GLObjects/Mesh.hpp"
#include "Renderer/GLObjects/Texture.hpp"
#include "Renderer/GLObjects/SourceShader.hpp"

#include "Loader.hpp"

#include <unordered_map>
#include <string>

namespace neo {

    class Engine;
    class Framebuffer;
    class SourceShader;

    class Library {

        friend Engine;

        public:
            Library() = default;
            ~Library() = default;
            Library(const Library&) = delete;
            Library& operator=(const Library&) = delete;

            static void init();
            static void tick();
            static void clean();

            static MeshData getMesh(const std::string&);
            static MeshData loadMesh(const std::string&, bool = false);
            static void insertMesh(const std::string&, MeshData& mesh);
            static const std::unordered_map<std::string, MeshData> getAllMeshes() { return mMeshes; }

            template <typename T>
            static Texture* createEmptyTexture(const std::string&, TextureFormat, glm::uvec3 = glm::uvec3(1));
            static Texture* getTexture(const std::string&);
            static Texture* loadTexture(const std::string&, TextureFormat = TextureFormat{});
            static Texture* loadCubemap(const std::string&, const std::vector<std::string> &);

            static Framebuffer* createFBO(const std::string&);
            static Framebuffer* createTransientFBO(glm::uvec2 size, const std::vector<TextureFormat>& formats);
            static Framebuffer* getFBO(const std::string&);

			static SourceShader* createSourceShader(const std::string& name, const SourceShader::ConstructionArgs& args);
			static SourceShader* createSourceShader(const std::string& name, const SourceShader::ShaderCode& shaderCode);
            static SourceShader* getSourceShader(const char* name);
            static const ResolvedShaderInstance& getDummyShader();

            static void imGuiEditor();

        private:
            static std::unordered_map<std::string, MeshData> mMeshes;
            static std::unordered_map<std::string, Texture*> mTextures;
            static std::unordered_map<std::string, Framebuffer*> mFramebuffers;
            struct TransientValue {
                Framebuffer* mFBO = nullptr;
                uint8_t mFrameCount = 0;
                bool mUsedThisFrame = false;
            };
            static std::unordered_map<uint32_t, std::vector<TransientValue>> mTransientFramebuffers;
            static std::unordered_map<std::string, SourceShader*> mShaders;
            static SourceShader* mDummyShader;

            static void _insertMesh(const std::string&, MeshData);
            static void _insertTexture(const std::string&, Texture*);
            static uint32_t _getTransientFBOHash(glm::uvec2 size, const std::vector<TextureFormat>& formats);
            static TransientValue& _findTransientResource(glm::uvec2 size, const std::vector<TextureFormat>& formats);
    };

    template <typename T>
    Texture* Library::createEmptyTexture(const std::string& name, TextureFormat format, glm::uvec3 size) {
        static_assert(std::is_base_of<Texture, T>::value, "T must be a Texture type");
        static_assert(!std::is_same<T, Texture>::value, "T must be a derived Texture type");

        auto it = mTextures.find(name);
        NEO_ASSERT(it == mTextures.end(), "Texture already found");
        float fd[] = { 1.f, 1.f, 1.f, 1.f };
        uint32_t id = 0xFFFFFFFF;
        void* data = nullptr;
        if (format.mType == GL_UNSIGNED_BYTE || format.mType == GL_INT) {
            data = &id;
        }
        else if (format.mType == GL_FLOAT) {
            data = fd;
        }
        else {
            NEO_FAIL("Trying to create an empty texture with an unsupported type");
        }
        Texture* t = new T(format, size, data);
        _insertTexture(name, t);
        return t;
    }


}



