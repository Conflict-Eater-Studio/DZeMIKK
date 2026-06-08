#include "enemySystem/behaviorTree/healthNodes.h"

bool game::HighHealthNode::evaluate(Context& ctx, ContextModifiers& modifiers) {
    if (ctx.hp <= 0.9f) {
        return false;
    }

    modifiers.attack *= 1.2f;
    modifiers.defense *= 1.2f;
    modifiers.heal *= 0.0f;

    return true;
}

bool game::MidHealthNode::evaluate(Context& ctx, ContextModifiers&) {
    return ctx.hp >= 0.3f && ctx.hp <= 0.9f;
}

bool game::LowHealthNode::evaluate(Context& ctx, ContextModifiers& modifiers) {
    if (ctx.hp >= 0.3f) {
        return false;
    }

    modifiers.attack *= 0.5f;
    modifiers.defense *= 1.5f;
    modifiers.heal *= 4.0f;

    return true;
}
