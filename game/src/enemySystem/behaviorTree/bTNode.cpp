#include "enemySystem/behaviorTree/bTNode.h"

game::SelectorNode::SelectorNode(std::vector<std::unique_ptr<BTNode>> children)
    : _children(std::move(children)) {}

bool game::SelectorNode::evaluate(Context& ctx, ContextModifiers& modifiers) {
    for (auto& child : _children) {

        if (child->evaluate(ctx, modifiers)) {
            return true;
        }
    }

    return false;
}

game::SequenceNode::SequenceNode(std::vector<std::unique_ptr<BTNode>> children)
    : _children(std::move(children)) {}

bool game::SequenceNode::evaluate(Context& ctx, ContextModifiers& modifiers) {
    for (auto& child : _children) {

        if (!child->evaluate(ctx, modifiers)) {
            return false;
        }
    }

    return true;
}
