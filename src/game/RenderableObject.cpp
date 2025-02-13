#include "RenderableObject.hpp"

void RenderableObject::SetPosition(float x, float y) {
    destination_rect_.x = x;
    destination_rect_.y = y;
}

void RenderableObject::SetX(float x) {
    destination_rect_.x = x;
}

void RenderableObject::SetY(float y) {
    destination_rect_.y = y;
}

void RenderableObject::SetSize(float width, float height) {
    destination_rect_.w = width;
    destination_rect_.h = height;
}