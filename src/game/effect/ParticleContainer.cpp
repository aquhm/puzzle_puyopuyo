#include "ParticleContainer.hpp"
#include "Particle.hpp"
#include "../../texture/ImageTexture.hpp"


    void ParticleAliveChecker::operator()(const std::unique_ptr<Particle>& particle)
    {
        if (particle && particle->IsAlive())
        {
            ++alive_count_;
        }
    }

    ParticleContainer::~ParticleContainer()
    {
        Release();
    }

    void ParticleContainer::Release()
    {
        ClearParticles();
        source_texture_.reset();
    }

    int ParticleContainer::GetAliveParticleCount() const
    {
        ParticleAliveChecker checker;
        checker = std::for_each(particles_.begin(), particles_.end(), checker);
        return checker.GetAliveCount();
    }

    void ParticleContainer::RemoveDeadParticles()
    {
        particles_.erase(std::remove_if(particles_.begin(), particles_.end(),
            [](const auto& particle)
            {
                return !particle || !particle->IsAlive();
            }),
            particles_.end()
        );
    }

    void ParticleContainer::ClearParticles()
    {
        particles_.clear();
    }
