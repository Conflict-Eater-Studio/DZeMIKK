#ifndef DZEMIKK_FONT_H
#define DZEMIKK_FONT_H

#include <ft2build.h>
#include <glm/ext/vector_int2.hpp>
#include <map>
#include <string>
#include <unordered_set>
#include <vector>
#include FT_FREETYPE_H

#include "assetManager/iGpuUploadable.h"

#include <glad/glad.h>
#include <iostream>

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

    std::map<unsigned int, Character> characters;
    std::map<unsigned int, PendingCharacter> pending;

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

        std::vector<unsigned int> glyphs;

        for (unsigned int c = 32; c < 128; c++)
            glyphs.push_back(c);

        unsigned int polish[] = {
            260, 261, // ¥ ¹
            262, 263, // Æ æ
            280, 281, // Ê ê
            321, 322, // £ ³
            323, 324, // Ñ ñ
            211, 243, // Ó ó
            346, 347, // Œ œ
            377, 378, //  Ÿ
            379, 380  // ¯ ¿
        };

        glyphs.insert(glyphs.end(), std::begin(polish), std::end(polish));

        for (unsigned int c : glyphs) {

            if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
                std::cout << "Failed glyph: " << c << std::endl;
                continue;
            }

            auto& bmp = face->glyph->bitmap;

            PendingCharacter pc;
            pc.width = static_cast<int>(bmp.width);
            pc.height = static_cast<int>(bmp.rows);
            pc.bearing = {face->glyph->bitmap_left, face->glyph->bitmap_top};
            pc.advance = face->glyph->advance.x;

            if (bmp.buffer && bmp.width > 0 && bmp.rows > 0) {
                pc.buffer.resize(static_cast<size_t>(bmp.width) * bmp.rows);

                for (unsigned int y = 0; y < bmp.rows; ++y) {
                    std::copy(bmp.buffer + y * bmp.pitch, bmp.buffer + y * bmp.pitch + bmp.width,
                              pc.buffer.begin() + y * bmp.width);
                }
            }

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

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, pc.width, pc.height, 0, GL_RED, GL_UNSIGNED_BYTE,
                         pc.buffer.data());

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