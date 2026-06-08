#ifndef GAME_BT_NODE_H
#define GAME_BT_NODE_H

#include <memory>
#include <vector>

namespace game {

/**
 * @brief Base class for behavior tree nodes.
 *
 * Behavior tree nodes evaluate combat context and modify decision-making
 * parameters used by enemy AI.
 */
class BTNode {
  public:
    /**
     * @brief Contains information about the current combat state.
     */
    struct Context {
        float hp = 0.0F;
        float attackRatio = 0.0F;
        float defenseRatio = 0.0F;
    };

    /**
     * @brief Contains modifiers produced during behavior tree evaluation.
     *
     * These values influence enemy pattern selection and prioritization.
     */
    struct ContextModifiers {
        float attack = 1.0F;
        float defense = 1.0F;
        float heal = 1.0F;
    };

    /**
     * @brief Evaluates the node.
     *
     * @param ctx Current combat context.
     * @param modifiers Modifiers updated during evaluation.
     *
     * @return true If the node succeeds.
     * @return false If the node fails.
     */
    virtual bool evaluate(Context& ctx, ContextModifiers& modifiers) = 0;

    virtual ~BTNode() = default;
};

/**
 * @brief Behavior tree selector node.
 *
 * Evaluates child nodes in order and succeeds when the first child succeeds.
 */
class SelectorNode final : public BTNode {
  public:
    /**
     * @brief Constructs a selector node.
     *
     * @param children Child nodes evaluated by the selector.
     */
    explicit SelectorNode(std::vector<std::unique_ptr<BTNode>> children);

    /**
     * @brief Evaluates the selector node.
     *
     * @param ctx Current combat context.
     * @param modifiers Modifiers updated during evaluation.
     *
     * @return true If any child succeeds.
     * @return false If all children fail.
     */
    bool evaluate(Context& ctx, ContextModifiers& modifiers) override;

  private:
    /**
     * @brief Child nodes evaluated by the selector.
     */
    std::vector<std::unique_ptr<BTNode>> _children;
};

/**
 * @brief Behavior tree sequence node.
 *
 * Evaluates child nodes in order and succeeds only if all children succeed.
 */
class SequenceNode final : public BTNode {
  public:
    /**
     * @brief Constructs a sequence node.
     *
     * @param children Child nodes evaluated by the sequence.
     */
    explicit SequenceNode(std::vector<std::unique_ptr<BTNode>> children);

    /**
     * @brief Evaluates the sequence node.
     *
     * @param ctx Current combat context.
     * @param modifiers Modifiers updated during evaluation.
     *
     * @return true If all children succeed.
     * @return false If any child fails.
     */
    bool evaluate(Context& ctx, ContextModifiers& modifiers) override;

  private:
    /**
     * @brief Child nodes evaluated by the sequence.
     */
    std::vector<std::unique_ptr<BTNode>> _children;
};

} // namespace game

#endif