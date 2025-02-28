#include "RenderableObject.hpp"

void RenderableObject::SetPosition(float x, float y) 
{
	position_.x = x;
    position_.y = y;

    destination_rect_.x = x;
    destination_rect_.y = y;
}

void RenderableObject::SetX(float x) 
{
    position_.x = x;

    destination_rect_.x = x;
}

void RenderableObject::SetY(float y) 
{
    position_.y = y;

    destination_rect_.y = y;
}

void RenderableObject::SetScale(float width, float height)
{
    size_.x = width;
    size_.y = height;

    destination_rect_.w = width;
    destination_rect_.h = height;
}