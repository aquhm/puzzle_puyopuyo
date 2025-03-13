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
    source_rect_ = 
    {
        0, 0,
        Constants::Particle::Explosion::SIZE,
        Constants::Particle::Explosion::SIZE,
    };
}

bool ExplosionContainer::InitializeParticles() 
{
    source_texture_ = ImageTexture::Create("PUYO/puyo_beta.png");
    if (!source_texture_)
    {
        assert(false && "Failed to load explosion texture");
        return false;
    }

    initial_lifetime_ = Constants::Particle::Explosion::DEFAULT_LIFETIME;
    accumulated_lifetime_ = 0.0f;

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
        particle->SetScale(size, size);

        particle->lifetime_ = 0.0f;
        particle->is_alive_ = true;

        particles_.push_back(std::move(particle));
    }

    return true;
}

void ExplosionContainer::UpdateParticlePhysics(ExplosionParticle& particle, float deltaTime) 
{

    particle.lifetime_ += deltaTime;

    if (particle.lifetime_ >= initial_lifetime_)
    {
        particle.is_alive_ = false;
        return;
    }

    float particleTime = particle.lifetime_;
    particle.velocity_ = 
    {
        particle.direction_.x * particleTime,
        particle.direction_.y * particleTime +
        (Constants::Particle::Explosion::GRAVITY / 2.0f * particleTime * particleTime)
    };

    SDL_FPoint newPosition
    {
        position_.x + particle.velocity_.x,
        position_.y + particle.velocity_.y
    };

    particle.SetPosition(newPosition.x, newPosition.y);
}

void ExplosionContainer::Update(float deltaTime) 
{
    for (auto& particle : particles_) 
    {
        if (auto explosionParticle = dynamic_cast<ExplosionParticle*>(particle.get())) 
        {
            UpdateParticlePhysics(*explosionParticle, deltaTime);
        }
    }
}

void ExplosionContainer::Render()
{
    if (!source_texture_ || particles_.empty()) 
    {
        return;
    }

    bool hasVisibleParticles = false;
    for (const auto& particle : particles_) 
    {
        if (particle && particle->IsAlive()) 
        {
            hasVisibleParticles = true;
            break;
        }
    }

    if (!hasVisibleParticles) 
    {
        return;
    }

    for (const auto& particle : particles_) 
    {
        if (particle && particle->IsAlive()) 
        {
            SDL_FRect destRect = particle->GetRect();

            source_texture_->RenderScaled(&source_rect_, &destRect);
        }
    }
}

void ExplosionContainer::SetBlockType(BlockType type) 
{
    type_ = type;

    const int particleX = Constants::Particle::Explosion::TEXTURE_POS_X;
    const int particleY = Constants::Particle::Explosion::TEXTURE_POS_Y;
    const int spacing = Constants::Particle::Explosion::TEXTURE_SPACING;

    switch (type) 
    {
    case BlockType::Red:
        source_rect_.x = particleX;
        source_rect_.y = particleY;
        break;
    case BlockType::Green:
        source_rect_.x = particleX + (Constants::Particle::Explosion::SIZE + spacing);
        source_rect_.y = particleY;
        break;
    case BlockType::Blue:
        source_rect_.x = particleX + (Constants::Particle::Explosion::SIZE + spacing) * 2;
        source_rect_.y = particleY;
        break;
    case BlockType::Yellow:
        source_rect_.x = particleX + (Constants::Particle::Explosion::SIZE + spacing) * 3;
        source_rect_.y = particleY;
        break;
    case BlockType::Purple:
        source_rect_.x = particleX + (Constants::Particle::Explosion::SIZE + spacing) * 4;
        source_rect_.y = particleY;
        break;
    }
}