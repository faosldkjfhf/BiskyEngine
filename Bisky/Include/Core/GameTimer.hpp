#pragma once

#include "Common.hpp"

namespace bisky::core
{
class GameTimer
{
  public:
    GameTimer();

    float gameTime() const;
    float deltaTime() const;

    void reset();
    void start();
    void stop();
    void tick();

  private:
    double m_secondsPerCount;
    double m_deltaTime;

    uint64_t m_baseTime;
    uint64_t m_pausedTime;
    uint64_t m_stopTime;
    uint64_t m_prevTime;
    uint64_t m_currTime;

    bool m_stopped;
};

} // namespace bisky::core