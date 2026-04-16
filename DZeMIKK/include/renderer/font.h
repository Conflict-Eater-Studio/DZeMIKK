#ifndef DZEMIKK_FONT_H
#define DZEMIKK_FONT_H

#include <glm/ext/vector_int2.hpp>
#include <map>
#include <string>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <glad/glad.h>

namespace dzemikk {

	struct Character {
        unsigned int textureID;
        glm::ivec2 size;
        glm::ivec2 bearing;
        unsigned int advance;
    };

	class Font {
      public:
        std::map<char, Character> characters;

        void clear() {
            for (auto& [c, ch] : characters) {
                glDeleteTextures(1, &ch.textureID);
            }
            characters.clear();
        }

        bool load(const std::string& path) {
            clear(); 

            FT_Library ft;
            if (FT_Init_FreeType(&ft))
                return false;

            FT_Face face;
            if (FT_New_Face(ft, path.c_str(), 0, &face))
                return false;

            FT_Set_Pixel_Sizes(face, 0, 48);

            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

            for (unsigned char c = 0; c < 128; c++) {
                if (FT_Load_Char(face, c, FT_LOAD_RENDER))
                    continue;

                unsigned int texture;
                glGenTextures(1, &texture);
                glBindTexture(GL_TEXTURE_2D, texture);

                glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width,
                             face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE,
                             face->glyph->bitmap.buffer);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                Character ch = {texture,
                                {face->glyph->bitmap.width, face->glyph->bitmap.rows},
                                {face->glyph->bitmap_left, face->glyph->bitmap_top},
                                face->glyph->advance.x};

                characters.insert({c, ch});
            }

            FT_Done_Face(face);
            FT_Done_FreeType(ft);

            return true;
        }
    };

} // namespace dzemikk
#endif // DZEMIKK_FONT_H
