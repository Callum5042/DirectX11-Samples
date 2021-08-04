#pragma once

class Timer
{
public:
	Timer();
	virtual ~Timer();

	virtual void Start();
	virtual void Stop();
	virtual void Reset();

	virtual void Tick();

	virtual double DeltaTime();
	virtual double TotalTime();

	constexpr bool IsActive() { return m_Active; }

protected:
	double m_SecondsPerCount = 0.0;
	double m_DeltaTime = 0.0;

	__int64 m_BaseTime = 0;
	__int64 m_PausedTime = 0;
	__int64 m_StopTime = 0;
	__int64 m_PrevTime = 0;
	__int64 m_CurrTime = 0;

	bool m_Active = false;
	bool m_Stopped = false;
};