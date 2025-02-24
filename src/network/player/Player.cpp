#include "Player.hpp"

void Player::Initialize() 
{
    id_ = 0;
    character_id_ = -1;
    net_info_ = nullptr;
}

void Player::Release() 
{
    net_info_ = nullptr;
}
