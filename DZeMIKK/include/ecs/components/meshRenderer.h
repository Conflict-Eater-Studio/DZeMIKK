#include "monoBehaviour.h"

namespace dzemikk {
class Transform;
class Mesh;
class Material;

class MeshRenderer : public MonoBehaviour {
  public:
    Mesh* mesh = nullptr;
    Material* material = nullptr;
    Transform* transform = nullptr;

    [[nodiscard]] std::string typeName() const override {
        return "MeshRenderer";
    };

    void start() override;

    void onDestroy() override;
};

} // namespace dzemikk