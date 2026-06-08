#pragma once

#include <memory>
#include <vector>

namespace game {

class BTNode {
  public:
    struct Context {
        float hp = 0.0F;
        float attackRatio = 0.0F;
        float defenseRatio = 0.0F;
    };

    struct ContextModifiers {
        float attack = 1.0F;
        float defense = 1.0F;
        float heal = 1.0F;
    };

    virtual bool evaluate(Context& ctx, ContextModifiers& modifiers) = 0;
    virtual ~BTNode() = default;
};


class SelectorNode final : public BTNode {
  public:
    explicit SelectorNode(std::vector<std::unique_ptr<BTNode>> children);

    bool evaluate(Context& ctx, ContextModifiers& modifiers) override;

  private:
    std::vector<std::unique_ptr<BTNode>> _children;
};


class SequenceNode final : public BTNode {
  public:
    explicit SequenceNode(std::vector<std::unique_ptr<BTNode>> children);

    bool evaluate(Context& ctx, ContextModifiers& modifiers) override;

  private:
    std::vector<std::unique_ptr<BTNode>> _children;
};
}