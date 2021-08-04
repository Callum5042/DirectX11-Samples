#include "Timer.h"
#include <SDL_timer.h>

Timer::Timer()
{
	__int64 countsPerSec = SDL_GetPerformanceFrequency();
	m_SecondsPerCount = 1.0 / static_cast<double>(countsPerSec);

	Reset();
}

Timer::~Timer()
{
}

void Timer::Start()
{
	__int64 startTime = SDL_GetPerformanceCounter();
	m_Active = true;

	if (m_Stopped)
	{
		m_PausedTime += (startTime - m_StopTime);

		m_PrevTime = startTime;
		m_StopTime = 0;
		m_Stopped = false;
	}
}

void Timer::Stop()
{
	if (!m_Stopped)
	{
		__int64 currTime = SDL_GetPerformanceCounter();

		m_StopTime = currTime;
		m_Stopped = true;
	}
}

void Timer::Reset()
{
	__int64 currTime = SDL_GetPerformanceCounter();

	m_BaseTime = currTime;
	m_PrevTime = currTime;
	m_StopTime = 0;
	m_Stopped = false;
	m_Active = false;
}

void Timer::Tick()
{
	if (m_Stopped)
	{
		m_DeltaTime = 0.0;
		return;
	}

	__int64 currTime = SDL_GetPerformanceCounter();
	m_CurrTime = currTime;

	// Time difference between this frame and the previous.
	m_DeltaTime = (m_CurrTime - m_PrevTime) * m_SecondsPerCount;

	// Prepare for next frame.
	m_PrevTime = m_CurrTime;

	if (m_DeltaTime < 0.0)
	{
		m_DeltaTime = 0.0;
	}
}

double Timer::DeltaTime()
{
	return static_cast<double>(m_DeltaTime);
}

double Timer::TotalTime()
{
	if (m_Stopped)
	{
		return static_cast<double>(((m_StopTime - m_PausedTime) - m_BaseTime) * m_SecondsPerCount);
	}
	else
	{
		return static_cast<double>(((m_CurrTime - m_PausedTime) - m_BaseTime) * m_SecondsPerCount);
	}
}
