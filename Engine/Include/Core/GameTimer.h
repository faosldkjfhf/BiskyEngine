#pragma once

#include "Common.h"

namespace Core
{

class GameTimer
{
public:
  GameTimer();

  float GameTime() const;
  float DeltaTime() const;

  void Reset();
  void Start();
  void Stop();
  void Tick();

private:
  double mSecondsPerCount;
  double mDeltaTime;

  UINT64 mBaseTime;
  UINT64 mPausedTime;
  UINT64 mStopTime;
  UINT64 mPrevTime;
  UINT64 mCurrTime;

  bool mStopped;
};

} // namespace Core