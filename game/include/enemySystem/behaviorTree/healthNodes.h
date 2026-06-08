#include "btNode.h"

namespace game {
class HighHealthNode final : public BTNode {
  public:
    using Context = BTNode::Context;
using ContextModifiers = BTNode::ContextModifiers;
    bool evaluate(Context& ctx, ContextModifiers& modifiers) override;
};

class MidHealthNode final : public BTNode {
  public:
      using Context = BTNode::Context;
using ContextModifiers = BTNode::ContextModifiers;
    bool evaluate(Context& ctx, ContextModifiers&) override;
};

class LowHealthNode final : public BTNode {
  public:
      using Context = BTNode::Context;
using ContextModifiers = BTNode::ContextModifiers;
    bool evaluate(Context& ctx, ContextModifiers& modifiers) override;
};
}
