#ifndef DZEMIKK_TEXTUPDATER_H
#define DZEMIKK_TEXTUPDATER_H

namespace dzemikk {
class TextUpdater : public MonoBehaviour {
public:
    using Base = MonoBehaviour;
    TextRenderer* text = nullptr;
    float time = 0.0f;

    void update(double deltaTime) override {
        time += deltaTime;
        text->text = "Time: " + std::to_string((int)time);
    }

    [[nodiscard]] std::string typeName() const override {
        return "TextUpdater";
    };
};
}
#endif
