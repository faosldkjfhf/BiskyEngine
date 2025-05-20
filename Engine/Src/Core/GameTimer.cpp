#include "Common.h"

#include "Core/GameTimer.h"

namespace Core
{

GameTimer::GameTimer()
    : mSecondsPerCount(0.0), mDeltaTime(-1.0), mBaseTime(0), mPausedTime(0), mPrevTime(0), mCurrTime(0), mStopTime(0),
      mStopped(false)
{
  __int64 countsPerSec;
  QueryPerformanceFrequency((LARGE_INTEGER *)&countsPerSec);
  mSecondsPerCount = 1.0 / (double)countsPerSec;
}

float GameTimer::GameTime() const
{
  if (mStopped)
  {
    return static_cast<float>(((mStopTime - mPausedTime) - mBaseTime) * mSecondsPerCount);
  }

  return static_cast<float>(((mCurrTime - mPausedTime) - mBaseTime) * mSecondsPerCount);
}

float GameTimer::DeltaTime() const
{
  return static_cast<float>(mDeltaTime);
}

void GameTimer::Reset()
{
  __int64 currTime;
  QueryPerformanceCounter((LARGE_INTEGER *)&currTime);

  mBaseTime = currTime;
  mPrevTime = currTime;
  mStopTime = 0;
  mStopped = false;
}

void GameTimer::Start()
{
  __int64 startTime;
  QueryPerformanceCounter((LARGE_INTEGER *)&startTime);

  if (mStopped)
  {
    mPausedTime += (startTime - mStopTime);
    mPrevTime = startTime;
    mStopTime = 0;
    mStopped = false;
  }
}

void GameTimer::Stop()
{
  if (!mStopped)
  {
    __int64 currTime;
    QueryPerformanceCounter((LARGE_INTEGER *)&currTime);

    mStopTime = currTime;
    mStopped = true;
  }
}

void GameTimer::Tick()
{
  if (mStopped)
  {
    mDeltaTime = 0.0;
    return;
  }

  __int64 currTime;
  QueryPerformanceCounter((LARGE_INTEGER *)&currTime);
  mCurrTime = currTime;
  mDeltaTime = (mCurrTime - mPrevTime) * mSecondsPerCount;

  mPrevTime = mCurrTime;
  if (mDeltaTime < 0.0)
  {
    mDeltaTime = 0.0;
  }
}

} // namespace Core