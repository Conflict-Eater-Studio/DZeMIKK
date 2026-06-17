#ifndef ITEM_ENTITY_H
#define ITEM_ENTITY_H

#include "boost/uuid/random_generator.hpp"
#pragma once
#include "Entity.h"
#include "assetManager/assetHandle.h"
#include "map/HexPattern.h"
#include "renderer/model.h"

#include <boost/uuid/uuid.hpp>
#include <optional>

#include <ecs/gameobject.h>

namespace game {

enum class ItemType : uint8_t { Heal, RevealPattern, RevealHex, BonusHex };

struct ItemSpawnConfig {
    boost::uuids::uuid persistantId = boost::uuids::random_generator()();
    boost::uuids::uuid chunkId;
    ItemType type = ItemType::Heal;
    std::optional<float> healAmount = std::nullopt;
    std::optional<HexPattern> bonusPattern = std::nullopt;

    bool operator==(const ItemSpawnConfig&) const = default;
};

#pragma region JSON Serialization
// NOLINTBEGIN(readability-identifier-naming)
inline void to_json(nlohmann::json& j, const ItemSpawnConfig& config) {
    j = nlohmann::json{{"persistantId", boost::uuids::to_string(config.persistantId)},
                       {"chunkId", boost::uuids::to_string(config.chunkId)},
                       {"type", static_cast<uint8_t>(config.type)},
                       {"healAmount", config.healAmount},
                       {"bonusPattern", config.bonusPattern}};
}

inline void from_json(const nlohmann::json& j, ItemSpawnConfig& config) {
    if (!j.contains("persistantId") || !j.contains("chunkId") || !j.contains("type")) {
        throw std::runtime_error("Missing required fields in ItemSpawnConfig JSON");
    }

    config.persistantId = boost::uuids::string_generator()(j.at("persistantId").get<std::string>());
    config.chunkId = boost::uuids::string_generator()(j.at("chunkId").get<std::string>());
    config.type = static_cast<ItemType>(j.at("type").get<uint8_t>());
    if (j.contains("healAmount") && !j.at("healAmount").is_null()) {
        config.healAmount = j.at("healAmount").get<float>();
    }
    if (j.contains("bonusPattern") && !j.at("bonusPattern").is_null()) {
        config.bonusPattern = j.at("bonusPattern").get<HexPattern>();
    }
}
// NOLINTEND(readability-identifier-naming)
#pragma endregion

class ItemEntity : public Entity {
  public:
    using Base = Entity;
    using ItemType = game::ItemType;

    ItemEntity() = default;
    ItemEntity(ItemSpawnConfig config) : _config(std::move(config)) {}

    [[nodiscard]] std::string typeName() const override {
        return "ItemEntity";
    }

    void onEnter(HexCellPtr cell) override {
        setCell(cell);

        if (_itemType == ItemType::Heal) {
            getOwner()->transform()->setPosition(
                cell->getCoord().toWorldPosition(1.0F, 0.1F, cell->getHeight()) +
                glm::vec3(0.0F, 0.5F, 0.0F));
        } else if (_itemType == ItemType::BonusHex) {
            getOwner()->transform()->setPosition(
                cell->getCoord().toWorldPosition(1.0F, 0.1F, cell->getHeight()) +
                glm::vec3(0.0F, 0.6F, 0.0F));
        } else if (_itemType == ItemType::RevealHex) {
            getOwner()->transform()->setPosition(
                cell->getCoord().toWorldPosition(1.0F, 0.1F, cell->getHeight()) +
                glm::vec3(0.0F, 0.7F, 0.0F));
        } else if (_itemType == ItemType::RevealPattern) {
            getOwner()->transform()->setPosition(
                cell->getCoord().toWorldPosition(1.0F, 0.1F, cell->getHeight()) +
                glm::vec3(0.0F, 0.5F, 0.0F));
        }

    };
    void onExit() override {};

    void setItemType(ItemType type) {
        _itemType = type;
    }
    
    [[nodiscard]] ItemType getItemType() const {
        return _itemType;
    }

    void setConfig(const ItemSpawnConfig& config) {
        _config = config;
    }
    [[nodiscard]] const ItemSpawnConfig& getConfig() const {
        return _config;
    }

    void consume() {
        _consumed = true;
    }
    [[nodiscard]] bool isConsumed() const {
        return _consumed;
    }

    [[nodiscard]] nlohmann::json save() const override {
        auto j = Base::save();
        j["itemType"] = static_cast<uint8_t>(_itemType);
        j["consumed"] = _consumed;
        return j;
    }

  protected:
    ItemType _itemType{ItemType::Heal};
    bool _consumed{false};
    ItemSpawnConfig _config;
};
} // namespace game

#endif // ITEM_ENTITY_H
