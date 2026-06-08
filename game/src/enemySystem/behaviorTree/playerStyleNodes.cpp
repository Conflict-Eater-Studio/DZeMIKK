#include "enemySystem/behaviorTree/playerStyleNodes.h"

bool game::AggressivePlayerNode::evaluate(Context& ctx, ContextModifiers& modifiers) {
    if (ctx.attackRatio <= 0.5f) {
        return false;
    }

    modifiers.attack *= 0.8f;
    modifiers.defense *= 2.0f;
    modifiers.heal *= 1.2f;

    return true;
}

bool game::DefensivePlayerNode::evaluate(Context& ctx, ContextModifiers& modifiers) {
    if (ctx.defenseRatio <= 0.5f) {
        return false;
    }

    modifiers.attack *= 2.5f;
    modifiers.defense *= 0.7f;
    modifiers.heal *= 0.8f;

    return true;
}

bool game::NeutralPlayerNode::evaluate(Context&, ContextModifiers&) {
    return true;
}
