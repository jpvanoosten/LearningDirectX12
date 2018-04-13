#include <HighResolutionClock.h>

HighResolutionClock::HighResolutionClock()
    : m_DeltaTime(0)
    , m_TotalTime(0)
{
    m_T0 = std::chrono::high_resolution_clock::now();
}

void HighResolutionClock::Tick()
{
    auto t1 = std::chrono::high_resolution_clock::now();
    m_DeltaTime = t1 - m_T0;
    m_TotalTime += m_DeltaTime;
    m_T0 = t1;
}

void HighResolutionClock::Reset()
{
    m_T0 = std::chrono::high_resolution_clock::now();
    m_DeltaTime = std::chrono::high_resolution_clock::duration();
    m_TotalTime = std::chrono::high_resolution_clock::duration();
}

double HighResolutionClock::GetDeltaNanoseconds() const
{
    return m_DeltaTime.count() * 1.0;
}
double HighResolutionClock::GetDeltaMicroseconds() const
{
    return m_DeltaTime.count() * 1e-3;
}

double HighResolutionClock::GetDeltaMilliseconds() const
{
    return m_DeltaTime.count() * 1e-6;
}

double HighResolutionClock::GetDeltaSeconds() const
{
    return m_DeltaTime.count() * 1e-9;
}

double HighResolutionClock::GetTotalNanoseconds() const
{
    return m_TotalTime.count() * 1.0;
}

double HighResolutionClock::GetTotalMicroseconds() const
{
    return m_TotalTime.count() * 1e-3;
}

double HighResolutionClock::GetTotalMilliSeconds() const
{
    return m_TotalTime.count() * 1e-6;
}

double HighResolutionClock::GetTotalSeconds() const
{
    return m_TotalTime.count() * 1e-9;
}
