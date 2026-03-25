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

    void start() override;

    void onDestroy() override;

    };

}