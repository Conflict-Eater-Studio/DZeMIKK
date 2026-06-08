#include "btNode.h"

namespace game {
class AggressivePlayerNode final : public BTNode {
  public:
    bool evaluate(Context& ctx, ContextModifiers& modifiers) override;
};

class DefensivePlayerNode final : public BTNode {
  public:
    bool evaluate(Context& ctx, ContextModifiers& modifiers) override;
};


class NeutralPlayerNode final : public BTNode {
  public:
    bool evaluate(Context&, ContextModifiers&) override;
};

}