#pragma once

namespace dzemikk {
    class Mesh;
    class PrimitiveFactory {
      public:
        static Mesh* CreateCube();
        static Mesh* CreateQuad();
        static Mesh* CreateSphere();
        static Mesh* CreateCapsule();
    };
} // namespace dzemikk
