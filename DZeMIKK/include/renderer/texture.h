#ifndef DZEMIKK_TEXTURE_H
#define DZEMIKK_TEXTURE_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>

#include "assetManager/iGpuUploadable.h"

namespace dzemikk {

    /**
     * @brief GPU texture abstraction.
     *
     * Wraps OpenGL texture with metadata, type information
     */
    class Texture: public IGpuUploadable {
      public:
        /**
         * @brief Semantic type of texture used in material system.
         */
        enum class Type {
            Albedo,
            Normal,
            Roughness,
            Metallic,
            AO,
            Height,
            Emissive,
            UI,
            Skybox,
            Unknown
        };

        /**
         * @brief Texture filtering mode.
         */
        enum class Filter { Nearest, Linear, LinearMipmap };

        /**
         * @brief Texture wrapping mode.
         */
        enum class Wrap { Repeat, ClampToEdge, MirroredRepeat };

        Texture(unsigned char* data, int width, int height, int channels,
                bool generateMipmaps = true);
        virtual ~Texture();

        #pragma region Disable copy / move

        Texture(const Texture&) = delete;
        Texture(Texture&&) = delete;
        Texture& operator=(const Texture&) = delete;
        Texture& operator=(Texture&&) = delete;

        #pragma endregion

        #pragma region GPU binding

        /**
         * @brief Binds texture to given slot.
         */
        void bind(uint32_t slot = 0) const;

        /**
         * @brief Unbinds texture from slot.
         */
        static void unbind(uint32_t slot = 0);

        #pragma endregion

        #pragma region Settings

        void setFilter(Filter minFilter, Filter magFilter);
        void setWrap(Wrap sWrap, Wrap tWrap);

        void initFromData(unsigned char* data, int width, int height, int channels,
                          bool generateMipmaps = true);

        #pragma endregion

        #pragma region Getters

        [[nodiscard]] GLuint getId() const {
            return _id;
        }
        [[nodiscard]] int getWidth() const {
            return _width;
        }
        [[nodiscard]] int getHeight() const {
            return _height;
        }
        [[nodiscard]] int getChannels() const {
            return _channels;
        }
        [[nodiscard]] const std::string& getPath() const {
            return _path;
        }
        [[nodiscard]] Type getType() const {
            return _type;
        }

        #pragma endregion

        #pragma region Metadata

        void setType(Type type) {
            _type = type;
        }

        #pragma endregion

        void replaceTexture(GLuint newId, int width, int height, int channels);

        void uploadToGPU() override;

      private:
        GLuint _id = 0;

        int _width = 0;
        int _height = 0;
        int _channels = 0;
        std::vector<unsigned char> _data;
        bool _generateMipmaps;

        std::string _path;
        Type _type = Type::Unknown;

        GLenum convertFilter(Filter f) const;
        GLenum convertWrap(Wrap w) const;

        void release();

    };
    } // namespace dzemikk

#endif // DZEMIKK_TEXTURE_H