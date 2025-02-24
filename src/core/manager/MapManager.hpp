#pragma once
/*
 *
 * 설명: 퍼즐 게임 스테이지별 배경 맵들을 관리하는 클래스
 *
 */


#include "IManager.hpp"
#include <memory>
#include <unordered_map>
#include <vector>
#include <string_view>

class GameBackground;

enum class MapType : uint8_t
{
    GrassLand = 0,
    IceLand = 13
};

class MapManager final : public IManager 
{
public:

    using GameMapContainer = std::unordered_map<uint8_t, std::shared_ptr<GameBackground>>;

    MapManager() = default;
    ~MapManager() override = default;
    
    MapManager(const MapManager&) = delete;
    MapManager& operator=(const MapManager&) = delete;
    MapManager(MapManager&&) = delete;
    MapManager& operator=(MapManager&&) = delete;
    

    [[nodiscard]] bool Initialize() override;
    [[nodiscard]] std::string_view GetName() const override { return "MapManager"; }
    void Update(float deltaTime) override;
    void Release() override;    

    [[nodiscard]] std::shared_ptr<GameBackground> CreateMap(uint8_t index);
    [[nodiscard]] std::shared_ptr<GameBackground> GetRandomMap();
    [[nodiscard]] std::shared_ptr<GameBackground> GetCurrentMap() const { return current_map_; }
    bool RemoveMap(uint8_t index);

private:
    
    GameMapContainer game_maps_;
    std::vector<uint8_t> map_indices_;
    std::shared_ptr<GameBackground> current_map_;
};