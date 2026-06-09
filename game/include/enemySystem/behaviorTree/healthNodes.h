#ifndef GAME_HEALTH_NODE_H
#define GAME_HEALTH_NODE_H

#include "btNode.h"

namespace game {

/**
 * @brief Behavior tree node for enemies with high health.
 *
 * Applies modifiers that favor aggressive behavior when the enemy
 * has a high remaining health percentage.
 */
class HighHealthNode final : public BTNode {
  public:
    using Context = BTNode::Context;
    using ContextModifiers = BTNode::ContextModifiers;

    /**
     * @brief Evaluates the high-health condition.
     *
     * @param ctx Current combat context.
     * @param modifiers Modifiers updated during evaluation.
     *
     * @return true If the high-health condition is satisfied.
     * @return false Otherwise.
     */
    bool evaluate(Context& ctx, ContextModifiers& modifiers) override;
};

/**
 * @brief Behavior tree node for enemies with medium health.
 *
 * Applies balanced behavior when the enemy health is within
 * a moderate range.
 */
class MidHealthNode final : public BTNode {
  public:
    using Context = BTNode::Context;
    using ContextModifiers = BTNode::ContextModifiers;

    /**
     * @brief Evaluates the mid-health condition.
     *
     * @param ctx Current combat context.
     * @param modifiers Modifiers updated during evaluation.
     *
     * @return true If the mid-health condition is satisfied.
     * @return false Otherwise.
     */
    bool evaluate(Context& ctx, ContextModifiers& modifiers) override;
};

/**
 * @brief Behavior tree node for enemies with low health.
 *
 * Applies modifiers that favor defensive or healing behavior when
 * the enemy is close to defeat.
 */
class LowHealthNode final : public BTNode {
  public:
    using Context = BTNode::Context;
    using ContextModifiers = BTNode::ContextModifiers;

    /**
     * @brief Evaluates the low-health condition.
     *
     * @param ctx Current combat context.
     * @param modifiers Modifiers updated during evaluation.
     *
     * @return true If the low-health condition is satisfied.
     * @return false Otherwise.
     */
    bool evaluate(Context& ctx, ContextModifiers& modifiers) override;
};

} // namespace game

#endif