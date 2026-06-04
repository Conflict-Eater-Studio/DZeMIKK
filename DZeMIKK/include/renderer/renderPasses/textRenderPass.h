#ifndef DZEMIKK_TEXT_RENDER_PASS_H
#define DZEMIKK_TEXT_RENDER_PASS_H

#include "renderer/renderPasses/IRenderPass.h"
#include "renderer/shader.h"

#include <glad.h>

namespace dzemikk {

class TextRenderer;

/**
 * @brief Render pass responsible for rendering 2D text.
 *
 * This pass handles TextRenderer components and renders text using
 * a bitmap or atlas-based font system (typically signed distance fields
 * or pre-baked glyph textures).
 *
 * Text is rendered in screen-space using the UI camera and orthographic
 * projection, ensuring stable UI alignment regardless of scene transforms.
 */
class TextRenderPass : public IRenderPass {
  public:
    /**
     * @brief Constructs the text render pass.
     *
     * Initializes GPU resources required for text rendering,
     * including shader program and geometry buffers.
     */
    TextRenderPass();

    /**
     * @brief Executes text rendering stage.
     *
     * Responsibilities:
     * - collect all active TextRenderer components
     * - render text quads using batched geometry
     *
     * @param ctx Shared render context containing UI camera and projection.
     */
    void execute(RenderContext& ctx) override;

  private:
    /**
     * @brief List of text renderers to draw this frame.
     */
    std::vector<TextRenderer*> _texts;

    /**
     * @brief Shader used for rendering glyphs.
     */
    Shader* _textShader = nullptr;

    /**
     * @brief Vertex Array Object for text quad rendering.
     */
    GLuint _textVAO = 0;

    /**
     * @brief Vertex Buffer Object storing quad vertex data.
     */
    GLuint _textVBO = 0;

    /**
     * @brief Initializes and compiles text rendering shader.
     */
    void initTextShader();
};

} // namespace dzemikk

#endif // DZEMIKK_TEXT_RENDER_PASS_H