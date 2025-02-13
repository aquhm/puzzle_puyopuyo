// MapManager.hpp
#pragma once

#include "IManager.hpp"
#include <memory>
#include <unordered_map>
#include <vector>
#include <string_view>

// Forward declarations
class GameBackground;

class MapManager final : public IManager {
public:
    // Delete copy/move operations
    MapManager(const MapManager&) = delete;
    MapManager& operator=(const MapManager&) = delete;
    MapManager(MapManager&&) = delete;
    MapManager& operator=(MapManager&&) = delete;

    // Singleton accessor
    static MapManager& GetInstance();

    // IManager interface implementation
    [[nodiscard]] bool Initialize() override;
    void Update(float deltaTime) override;
    void Release() override;
    [[nodiscard]] std::string_view GetName() const override { return "MapManager"; }

    // Map management
    [[nodiscard]] std::shared_ptr<GameBackground> CreateMap(uint8_t index);
    [[nodiscard]] std::shared_ptr<GameBackground> GetRandomMap();
    bool RemoveMap(uint8_t index);

    // Getters
    [[nodiscard]] std::shared_ptr<GameBackground> GetCurrentMap() const { return current_map_; }

    // Map types enumeration
    enum class MapType : uint8_t {
        GrassLand = 0,
        IceLand = 13
    };

private:
    MapManager() = default;
    ~MapManager() override = default;

    using GameMapContainer = std::unordered_map<uint8_t, std::shared_ptr<GameBackground>>;
    GameMapContainer game_maps_;
    std::vector<uint8_t> map_indices_;
    std::shared_ptr<GameBackground> current_map_;
};