#pragma once

#include <vector>
#include <queue>
#include <memory>
#include <functional>

template<typename T>
class ObjectPool {
public:
    explicit ObjectPool(size_t initialSize = 100) {
        objects.reserve(initialSize);
        for (size_t i = 0; i < initialSize; ++i) {
            addNewObject();
        }
    }

    template<typename... Args>
    T* acquire(Args&&... args) {
        if (availableObjects.empty()) {
            addNewObject();
        }

        auto* object = availableObjects.front();
        availableObjects.pop();
        object->init(std::forward<Args>(args)...);
        return object;
    }

    void release(T* object) {
        object->reset();
        availableObjects.push(object);
    }

private:
    std::vector<std::unique_ptr<T>> objects;
    std::queue<T*> availableObjects;

    void addNewObject() {
        objects.push_back(std::make_unique<T>());
        availableObjects.push(objects.back().get());
    }
};
