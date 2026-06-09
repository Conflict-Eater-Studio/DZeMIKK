#pragma once

#include "btNode.h"

namespace game {

/**
 * @brief Behavior tree node for aggressive player behavior.
 *
 * Detects situations where the player favors offensive actions and
 * adjusts enemy decision-making accordingly.
 */
class AggressivePlayerNode final : public BTNode {
  public:
    /**
     * @brief Evaluates the aggressive-player condition.
     *
     * @param ctx Current combat context.
     * @param modifiers Modifiers updated during evaluation.
     *
     * @return true If the aggressive-player condition is satisfied.
     * @return false Otherwise.
     */
    bool evaluate(Context& ctx, ContextModifiers& modifiers) override;
};

/**
 * @brief Behavior tree node for defensive player behavior.
 *
 * Detects situations where the player favors defensive actions and
 * adjusts enemy decision-making accordingly.
 */
class DefensivePlayerNode final : public BTNode {
  public:
    /**
     * @brief Evaluates the defensive-player condition.
     *
     * @param ctx Current combat context.
     * @param modifiers Modifiers updated during evaluation.
     *
     * @return true If the defensive-player condition is satisfied.
     * @return false Otherwise.
     */
    bool evaluate(Context& ctx, ContextModifiers& modifiers) override;
};

/**
 * @brief Behavior tree node for neutral player behavior.
 *
 * Represents a balanced player strategy when neither offensive nor
 * defensive tendencies strongly dominate.
 */
class NeutralPlayerNode final : public BTNode {
  public:
    /**
     * @brief Evaluates the neutral-player condition.
     *
     * @param ctx Current combat context.
     * @param modifiers Modifiers updated during evaluation.
     *
     * @return true If the neutral-player condition is satisfied.
     * @return false Otherwise.
     */
    bool evaluate(Context& ctx, ContextModifiers& modifiers) override;
};

} // namespace game