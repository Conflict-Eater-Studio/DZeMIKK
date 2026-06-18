#pragma once
#ifndef GAME_DIALOG_ENTITY_H
#define GAME_DIALOG_ENTITY_H

#include "dialog.h"
#include "map/Entity.h"

namespace game {
struct DialogSpawnConfig {
    boost::uuids::uuid persistantId = boost::uuids::random_generator()();
    boost::uuids::uuid targetEntityId;
    std::vector<Dialog::Entry> entries;

    bool operator==(const DialogSpawnConfig&) const = default;
};

#pragma region JSON Serialization
// NOLINTBEGIN(readability-identifier-naming)
inline void to_json(nlohmann::json& j, const DialogSpawnConfig& config) {
    j = nlohmann::json{
        {"persistantId", boost::uuids::to_string(config.persistantId)},
        {"targetEntityId", boost::uuids::to_string(config.targetEntityId)},
        {"entries", config.entries},
    };
}

inline void from_json(const nlohmann::json& j, DialogSpawnConfig& config) {
    if (!j.contains("persistantId") || !j.contains("targetEntityId") ||
        !j.contains("entries")) {
        throw std::runtime_error("Missing required fields in DialogSpawnConfig JSON");
    }

    config.persistantId = boost::uuids::string_generator()(j.at("persistantId").get<std::string>());
    config.targetEntityId = boost::uuids::string_generator()(j.at("targetEntityId").get<std::string>());
    config.entries = j.at("entries").get<std::vector<Dialog::Entry>>();
}
// NOLINTEND(readability-identifier-naming)
#pragma endregion

class DialogEntity : public Entity {
  public:
    using Base = Entity;

    [[nodiscard]] std::string typeName() const override {
        return "DialogEntity";
    }

    void onEnter(HexCellPtr cell) override {};
    void onExit() override {};

    [[nodiscard]] const Dialog& getDialog() const {
        return _dialog;
    }
    [[nodiscard]] Dialog& getDialog() {
        return _dialog;
    }

  private:
    Dialog _dialog;
    DialogSpawnConfig _config;
};
} // namespace game

#endif // GAME_DIALOG_ENTITY_H
