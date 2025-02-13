#include "MapManager.hpp"
#include "GameBackground.hpp"
#include <random>
#include <chrono>
#include <algorithm>
#include <stdexcept>

MapManager& MapManager::GetInstance() {
    static MapManager instance;
    return instance;
}

bool MapManager::Initialize() {
    try {
        // Initialize map indices
        map_indices_.push_back(static_cast<uint8_t>(MapType::GrassLand));
        map_indices_.push_back(static_cast<uint8_t>(MapType::IceLand));
        return true;
    }
    catch (const std::exception& e) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MapManager initialization failed: %s", e.what());
        return false;
    }
}

void MapManager::Update(float deltaTime) {
    // Update current map if exists
    if (current_map_) {
        current_map_->Update(deltaTime);
    }
}

void MapManager::Release() {
    game_maps_.clear();
    current_map_.reset();
}

std::shared_ptr<GameBackground> MapManager::CreateMap(uint8_t index) {
    // Check if map already exists
    if (auto it = game_maps_.find(index); it != game_maps_.end()) {
        current_map_ = it->second;
        return current_map_;
    }

    // Create new map based on index
    std::shared_ptr<GameBackground> new_map;

    switch (static_cast<MapType>(index)) {
    case MapType::GrassLand:
        new_map = std::make_shared<GrassLand_BG>();
        break;
    case MapType::IceLand:
        new_map = std::make_shared<IceLand_BG>();
        break;
    default:
        throw std::runtime_error("Invalid map index");
    }

    if (!new_map->Initialize()) {
        throw std::runtime_error("Failed to initialize map");
    }

    current_map_ = new_map;
    game_maps_[index] = new_map;

    return new_map;
}

std::shared_ptr<GameBackground> MapManager::GetRandomMap() {
    if (map_indices_.empty()) {
        throw std::runtime_error("No maps available");
    }

    // Use modern random number generation
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, map_indices_.size() - 1);

    return CreateMap(map_indices_[dist(gen)]);
}

bool MapManager::RemoveMap(uint8_t index) {
    if (auto it = game_maps_.find(index); it != game_maps_.end()) {
        if (current_map_ == it->second) {
            current_map_.reset();
        }
        game_maps_.erase(it);
        return true;
    }
    return false;
}