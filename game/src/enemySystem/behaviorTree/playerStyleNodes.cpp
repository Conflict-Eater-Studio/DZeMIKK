#include "enemySystem/behaviorTree/playerStyleNodes.h"

bool game::AggressivePlayerNode::evaluate(Context& ctx, ContextModifiers& modifiers) {
    if (ctx.attackRatio <= 0.5F) {
        return false;
    }

    modifiers.attack *= 0.8F;
    modifiers.defense *= 2.0F;
    modifiers.heal *= 1.2F;

    return true;
}

bool game::DefensivePlayerNode::evaluate(Context& ctx, ContextModifiers& modifiers) {
    if (ctx.defenseRatio <= 0.5F) {
        return false;
    }

    modifiers.attack *= 2.5F;
    modifiers.defense *= 0.7F;
    modifiers.heal *= 0.8F;

    return true;
}

bool game::NeutralPlayerNode::evaluate(Context& ctx, ContextModifiers& modifiers) {
    return true;
}
