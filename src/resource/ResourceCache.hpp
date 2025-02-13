#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <chrono>
#include "TextureLoader.hpp"

template<typename T>
class ResourceCache {
public:
    struct CacheEntry {
        std::shared_ptr<T> resource;
        std::chrono::steady_clock::time_point lastUsed;
        size_t memorySize;
    };

    std::shared_ptr<T> get(const std::string& key) {
        auto it = cache.find(key);
        if (it != cache.end()) {
            it->second.lastUsed = std::chrono::steady_clock::now();
            return it->second.resource;
        }
        return nullptr;
    }

    void add(const std::string& key, std::shared_ptr<T> resource, size_t size) {
        if (currentMemoryUsage + size > maxMemoryUsage) {
            cleanup();
        }

        cache[key] = {
            resource,
            std::chrono::steady_clock::now(),
            size
        };
        currentMemoryUsage += size;
    }

private:
    std::unordered_map<std::string, CacheEntry> cache;
    size_t currentMemoryUsage{ 0 };
    const size_t maxMemoryUsage{ 1024 * 1024 * 512 }; // 512MB 기본 제한

    void cleanup() {
        auto now = std::chrono::steady_clock::now();
        std::vector<std::string> keysToRemove;

        for (const auto& [key, entry] : cache) {
            auto age = std::chrono::duration_cast<std::chrono::seconds>(
                now - entry.lastUsed).count();

            if (age > 300) { // 5분 이상 미사용
                keysToRemove.push_back(key);
                currentMemoryUsage -= entry.memorySize;
            }
        }

        for (const auto& key : keysToRemove) {
            cache.erase(key);
        }
    }
};