#include "ExplosionEffect.hpp"
#include "../../core/manager/ResourceManager.hpp"
#include "../../core/common/constants/Constants.hpp"
#include "../../core/GameApp.hpp"
#include "../../texture/ImageTexture.hpp"
#include "../../core/GameUtils.hpp"
#include "../../game/block/Block.hpp"

#include <cassert>

ExplosionParticle::ExplosionParticle() = default;

ExplosionContainer::ExplosionContainer() 
{
    sourceRect_ = 
    {
        0, 0,
        Constants::Particle::Explosion::SIZE,
        Constants::Particle::Explosion::SIZE,
    };

    explosionRect_ = 
    {
        0, 0,
        Constants::Block::SIZE,
        Constants::Block::SIZE
    };
}

bool ExplosionContainer::InitializeParticles() 
{
    sourceTexture_ = ImageTexture::Create("PUYO/puyo_beta.png");
    if (!sourceTexture_)
    {
        assert(false && "Failed to load explosion texture");
        return false;
    }

    initialLifetime_ = Constants::Particle::Explosion::DEFAULT_LIFETIME;
    accumulatedLifetime_ = 0.0f;

    particles_.clear();
    particles_.reserve(Constants::Particle::Explosion::PARTICLE_COUNT);

    for (size_t i = 0; i < Constants::Particle::Explosion::PARTICLE_COUNT; ++i)
    {
        auto particle = std::make_unique<ExplosionParticle>();

        particle->SetPosition(position_.x, position_.y);

        const float angle = GameUtils::Random::Range(25.0f, 155.0f);
        const float force = GameUtils::Random::Range(20.0f, 40.0f);

        particle->direction_ =
        {
            force * std::cos(GameUtils::ToRadians(angle)),
            force * -std::sin(GameUtils::ToRadians(angle))
        };

        const float size = static_cast<float>(GameUtils::Random::Range(7, 15)); // 7 ~ 15
        particle->SetSize(size);

        particle->lifetime_ = 0.0f;
        particle->isAlive_ = true;

        particles_.push_back(std::move(particle));
    }

    return true;
}

void ExplosionContainer::UpdateParticlePhysics(ExplosionParticle& particle, float deltaTime) 
{
    if (particle.lifetime_ >= initialLifetime_) 
    {
        particle.isAlive_ = false;
        return;
    }

    accumulatedLifetime_ += deltaTime;

    // 물리 계산
    particle.velocity_ = 
    {
        particle.direction_.x * accumulatedLifetime_,
        particle.direction_.y * accumulatedLifetime_ + (Constants::Particle::Explosion::GRAVITY / 2.0f * accumulatedLifetime_ * accumulatedLifetime_)
    };

    SDL_FPoint newPosition{ position_.x + particle.velocity_.x, position_.y + particle.velocity_.y };
    particle.SetPosition(newPosition.x, newPosition.y);

    particle.lifetime_ += deltaTime;
}

void ExplosionContainer::Update(float deltaTime) 
{
    for (auto& particle : particles_) 
    {
        if (auto* explosionParticle = dynamic_cast<ExplosionParticle*>(particle.get())) 
        {
            UpdateParticlePhysics(*explosionParticle, deltaTime);
        }
    }
}

void ExplosionContainer::Render()
{
    if (!sourceTexture_)
    {
        return;
    }

    for (const auto& particle : particles_) 
    {
        if (particle->IsAlive())
        {
            sourceTexture_->RenderScaled(&sourceRect_, &particle->GetRect());
        }
    }
}

SDL_FRect ExplosionContainer::GetTextureRectForType(BlockType type) const
{
    const int baseX = 1;
    const int baseY = 1;
    const int stride = static_cast<int>(Constants::Block::SIZE) + 1;

    SDL_FRect rect = std::move(explosionRect_);

    switch (type) {
    case BlockType::Red:
        rect.x = baseX + stride * 6;
        rect.y = baseY + stride * 10;
        break;
    case BlockType::Green:
        rect.x = baseX + stride * 8;
        rect.y = baseY + stride * 10;
        break;
    case BlockType::Blue:
        rect.x = baseX + stride * 10;
        rect.y = baseY + stride * 10;
        break;
    case BlockType::Yellow:
        rect.x = baseX + stride * 12;
        rect.y = baseY + stride * 10;
        break;
    case BlockType::Purple:
        rect.x = baseX + stride * 14;
        rect.y = baseY + stride * 10;
        break;
    }

    return rect;
}

void ExplosionContainer::SetBlockType(BlockType type) {
    type_ = type;

    const int particleX = Constants::Particle::Explosion::TEXTURE_POS_X;
    const int particleY = Constants::Particle::Explosion::TEXTURE_POS_Y;
    const int spacing = Constants::Particle::Explosion::TEXTURE_SPACING;

    // 파티클 텍스처 위치 설정
    switch (type) {
    case BlockType::Red:
        sourceRect_.x = particleX;
        sourceRect_.y = particleY;
        break;
    case BlockType::Green:
        sourceRect_.x = particleX + (Constants::Particle::Explosion::SIZE + spacing);
        sourceRect_.y = particleY;
        break;
    case BlockType::Blue:
        sourceRect_.x = particleX + (Constants::Particle::Explosion::SIZE + spacing) * 2;
        sourceRect_.y = particleY;
        break;
    case BlockType::Yellow:
        sourceRect_.x = particleX + (Constants::Particle::Explosion::SIZE + spacing) * 3;
        sourceRect_.y = particleY;
        break;
    case BlockType::Purple:
        sourceRect_.x = particleX + (Constants::Particle::Explosion::SIZE + spacing) * 4;
        sourceRect_.y = particleY;
        break;
    }

    explosionRect_ = GetTextureRectForType(type);
}