#ifndef DZEMIKK_MONOBEHAVIOUR_H
#define DZEMIKK_MONOBEHAVIOUR_H

#include "../component.h"

namespace dzemikk {
    class MonoBehaviour : public Component {
      public:
        virtual void start() {};
        virtual void update() {};
        virtual void lateUpdate() {};
        virtual void onDestroy() {};
    };
} 

#endif // DZEMIKK_MONOBEHAVIOUR_H