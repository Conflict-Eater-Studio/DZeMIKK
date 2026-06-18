#ifndef DZEMIKK_FONT_H
#define DZEMIKK_FONT_H

#include <glm/ext/vector_int2.hpp>
#include <map>
#include <string>
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <glad/glad.h>
#include <iostream>

#include "assetManager/iGpuUploadable.h"

namespace dzemikk {

	struct Character {
        unsigned int textureID;
        glm::ivec2 size;
        glm::ivec2 bearing;
        unsigned int advance;
    };

    struct PendingCharacter {
        std::vector<unsigned char> buffer;
        int width;
        int height;
        glm::ivec2 bearing;
        unsigned int advance;
    };

	class Font : public IGpuUploadable {
      public:
        virtual ~Font() = default; 
        std::map<char, Character> characters;
        std::map<char, PendingCharacter> pending;
        int lineHeight = 0;
        int baseSize = 0;

        void clear() {
            for (auto& [c, ch] : characters) {
                glDeleteTextures(1, &ch.textureID);
            }
            characters.clear();
            pending.clear();
        }

        bool load(const std::string& path) {
            clear();
            pending.clear();

            FT_Library ft;
            if (FT_Init_FreeType(&ft))
                return false;

            FT_Face face;
            if (FT_New_Face(ft, path.c_str(), 0, &face)) {
                FT_Done_FreeType(ft);
                return false;
            }

            if (FT_Set_Pixel_Sizes(face, 0, 48)) {
                FT_Done_Face(face);
                FT_Done_FreeType(ft);
                return false;
            }

            lineHeight = face->size->metrics.height >> 6;
            baseSize = 48;

            for (unsigned char c = 0; c < 128; c++) {

                if (FT_Load_Char(face, c, FT_LOAD_RENDER))
                    continue;

                auto& bmp = face->glyph->bitmap;

                if (!bmp.buffer)
                    continue;

                PendingCharacter pc;
                pc.buffer.assign(bmp.buffer, bmp.buffer + bmp.width * bmp.rows);

                pc.width = bmp.width;
                pc.height = bmp.rows;
                pc.bearing = {face->glyph->bitmap_left, face->glyph->bitmap_top};

                pc.advance = face->glyph->advance.x; 

                pending[c] = std::move(pc);
            }

            FT_Done_Face(face);
            FT_Done_FreeType(ft);

            return true;
        }

        void uploadToGPU() override {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

            for (auto& [c, pc] : pending) {

                unsigned int texture;
                glGenTextures(1, &texture);
                glBindTexture(GL_TEXTURE_2D, texture);

                glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, pc.width, pc.height, 0, GL_RED,
                             GL_UNSIGNED_BYTE, pc.buffer.data());

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                characters[c] = Character{texture, {pc.width, pc.height}, pc.bearing, pc.advance};
            }

            pending.clear();
        }
    };

} // namespace dzemikk
#endif // DZEMIKK_FONT_H
