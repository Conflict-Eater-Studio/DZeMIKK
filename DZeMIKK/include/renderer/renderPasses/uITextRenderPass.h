#ifndef DZEMIKK_UI_TEXT_RENDER_PASS_H
#define DZEMIKK_UI_TEXT_RENDER_PASS_H

#include "renderer/renderPasses/IRenderPass.h"
#include "renderer/shader.h"

#include <glad.h>

namespace dzemikk {

class UITextRenderer;

/**
 * @brief Render pass responsible for rendering UI text elements.
 *
 * This pass handles UITextRenderer components
 *
 */
class UITextRenderPass : public IRenderPass {
  public:
    /**
     * @brief Constructs UI text render pass.
     *
     * Initializes GPU resources required for rendering UI text,
     * including shader setup and buffer allocation.
     */
    UITextRenderPass();

    /**
     * @brief Executes UI text rendering stage.
     *
     * Responsibilities:
     * - gather all active UITextRenderer components
     * - prepare glyph quads for screen-space rendering
     * - render text using UI projection matrix
     *
     * @param ctx Shared render context containing UI camera and projection.
     */
    void execute(RenderContext& ctx) override;

  private:
    /**
     * @brief List of UI text renderers scheduled for rendering.
     */
    std::vector<UITextRenderer*> _texts;

    /**
     * @brief Shader used for UI text rendering.
     */
    Shader* _textShader = nullptr;

    /**
     * @brief Vertex Array Object for text quad geometry.
     */
    GLuint _textVAO = 0;

    /**
     * @brief Vertex Buffer Object storing quad vertex data.
     */
    GLuint _textVBO = 0;

    /**
     * @brief Initializes and compiles the UI text shader.
     */
    void initTextShader();
};

} // namespace dzemikk

#endif // DZEMIKK_UI_TEXT_RENDER_PASS_H