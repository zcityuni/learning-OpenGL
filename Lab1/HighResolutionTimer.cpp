#include "HighResolutionTimer.h"

CHighResolutionTimer::CHighResolutionTimer() :
m_bStarted(false)
{
}

CHighResolutionTimer::~CHighResolutionTimer()
{
}

void CHighResolutionTimer::Start()
{
	m_bStarted = true;
	QueryPerformanceCounter(&m_t1);
}

double CHighResolutionTimer::Elapsed()
{
	if (!m_bStarted)
		return 0.0;

	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);

	QueryPerformanceCounter(&m_t2);

	// Return the elapsed time in seconds
	return (double) (m_t2.QuadPart - m_t1.QuadPart) / frequency.QuadPart;
}