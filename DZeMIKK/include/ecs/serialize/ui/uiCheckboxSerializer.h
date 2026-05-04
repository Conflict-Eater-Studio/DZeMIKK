#pragma once
#ifndef DZEMIKK_UICHECKBOXSERIALIZER_H
#define DZEMIKK_UICHECKBOXSERIALIZER_H

#include "ecs/components/ui/uiCheckbox.h"
#include <nlohmann/json.hpp>

inline void to_json(nlohmann::json& json, const dzemikk::UICheckbox& button) {
    const auto normalColor = button.getNormalColor();
    const auto hoverColor = button.getHoverColor();
    const auto pressedColor = button.getPressedColor();

    json["type"] = button.typeName();
    json["id"] = boost::uuids::to_string(button.getId());
    json["normalColor"] = {normalColor[0], normalColor[1], normalColor[2], normalColor[3]};
    json["hoverColor"] = {hoverColor[0], hoverColor[1], hoverColor[2], hoverColor[3]};
    json["pressedColor"] = {pressedColor[0], pressedColor[1], pressedColor[2], pressedColor[3]};
    json["onClickActionId"] =
        button.getOnClickActionId().empty() ? "" : button.getOnClickActionId();
    json["onEnterActionId"] =
        button.getOnEnterActionId().empty() ? "" : button.getOnEnterActionId();
    json["onExitActionId"] = button.getOnExitActionId().empty() ? "" : button.getOnExitActionId();

}
#endif //TUL_PBL_DZEMIKK_UICHECKBOXSERIALIZER_H
