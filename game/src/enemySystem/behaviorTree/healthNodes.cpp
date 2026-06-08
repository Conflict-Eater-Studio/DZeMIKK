#include "enemySystem/behaviorTree/healthNodes.h"

bool game::HighHealthNode::evaluate(Context& ctx, ContextModifiers& modifiers) {
    if (ctx.hp <= 0.9F) {
        return false;
    }

    modifiers.attack *= 1.2F;
    modifiers.defense *= 1.2F;
    modifiers.heal *= 0.0F;

    return true;
}

bool game::MidHealthNode::evaluate(Context& ctx, ContextModifiers& modifiers) {
    return ctx.hp >= 0.3F && ctx.hp <= 0.9F;
}

bool game::LowHealthNode::evaluate(Context& ctx, ContextModifiers& modifiers) {
    if (ctx.hp >= 0.3F) {
        return false;
    }

    modifiers.attack *= 0.5F;
    modifiers.defense *= 1.5F;
    modifiers.heal *= 4.0F;

    return true;
}
