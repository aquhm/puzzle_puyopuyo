//#pragma once
//
//#include <memory>
//#include <vector>
//#include "UIComponent.hpp"
//#include "GameConfig.hpp"
//
//class GameUI {
//public:
//    enum class Layer {
//        Background,
//        Game,
//        HUD,
//        Overlay,
//        Modal
//    };
//
//    struct UIElement {
//        std::unique_ptr<UIComponent> component;
//        Layer layer;
//        bool isVisible{ true };
//    };
//
//    void addElement(std::unique_ptr<UIComponent> component, Layer layer);
//    void update(float deltaTime);
//    void render();
//    void handleEvent(const SDL_Event& event);
//
//    template<typename T>
//    T* getComponent(const std::string& id);
//
//private:
//    std::vector<UIElement> elements;
//    void sortElementsByLayer();
//};