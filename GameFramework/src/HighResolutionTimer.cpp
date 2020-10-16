#include "GameFrameworkPCH.h"

#include <GameFramework/HighResolutionTimer.h>

using namespace std::chrono;

#define WINDOWS_CLOCK         0
#define HIGH_RESOLUTION_CLOCK 1
#define STEADY_CLOCK          2

#ifndef USE_CLOCK
    #define USE_CLOCK HIGH_RESOLUTION_CLOCK
#endif

#if USE_CLOCK == WINDOWS_CLOCK
class HighResolutionTimer::impl
{
public:
    impl()
    : t0 { 0 }
    , t1 { 0 }
    , frequency { 0 }
    , elapsedNanoseconds( 0.0 )
    , totalNanseconds( 0.0 )
    {
        ::QueryPerformanceFrequency( &frequency );
        ::QueryPerformanceCounter( &t0 );
    }

    void Tick()
    {
        ::QueryPerformanceCounter( &t1 );

        elapsedNanoseconds = ( t1.QuadPart - t0.QuadPart ) * ( 1e9 / frequency.QuadPart );
        totalNanseconds += elapsedNanoseconds;

        t0 = t1;
    }

    void Reset()
    {
        ::QueryPerformanceCounter( &t0 );
        elapsedNanoseconds = 0.0;
        totalNanseconds    = 0.0;
    }

    double ElapsedNanoseconds() const
    {
        return elapsedNanoseconds;
    }

    double TotalNanoseconds() const
    {
        return totalNanseconds;
    }

private:
    LARGE_INTEGER t0, t1;
    LARGE_INTEGER frequency;
    double        elapsedNanoseconds;
    double        totalNanseconds;
};
#elif USE_CLOCK == HIGH_RESOLUTION_CLOCK
class HighResolutionTimer::impl
{
public:
    impl()
    : elapsedTime( 0.0 )
    , totalTime( 0.0 )
    {
        t0 = high_resolution_clock::now();
    }

    void Tick()
    {
        t1                                = high_resolution_clock::now();
        duration<double, std::nano> delta = t1 - t0;
        t0                                = t1;
        elapsedTime                       = delta.count();
        totalTime += elapsedTime;
    }

    void Reset()
    {
        t0          = high_resolution_clock::now();
        elapsedTime = 0.0;
        totalTime   = 0.0;
    }

    double ElapsedNanoseconds() const
    {
        return elapsedTime;
    }

    double TotalNanoseconds() const
    {
        return totalTime;
    }

private:
    high_resolution_clock::time_point t0, t1;
    double                            elapsedTime;
    double                            totalTime;
};
#elif USE_CLOCK == STEADY_CLOCK
class HighResolutionTimer::impl
{
public:
    impl()
    : elapsedTime( 0.0 )
    , totalTime( 0.0 )
    {
        t0 = steady_clock::now();
    }

    void Tick()
    {
        t1                                = steady_clock::now();
        duration<double, std::nano> delta = t1 - t0;
        t0                                = t1;
        elapsedTime                       = delta.count();
        totalTime += elapsedTime;
    }

    void Reset()
    {
        t0          = steady_clock::now();
        elapsedTime = 0.0;
        totalTime   = 0.0;
    }

    double ElapsedNanoseconds() const
    {
        return elapsedTime;
    }

    double TotalNanoseconds() const
    {
        return totalTime;
    }

private:
    steady_clock::time_point t0, t1;
    double                   elapsedTime;
    double                   totalTime;
};
#endif

HighResolutionTimer::HighResolutionTimer()
{
    pImpl = std::make_unique<impl>();
}

HighResolutionTimer::~HighResolutionTimer() {}

void HighResolutionTimer::Tick()
{
    pImpl->Tick();
}

void HighResolutionTimer::Reset()
{
    pImpl->Reset();
}

double HighResolutionTimer::ElapsedSeconds() const
{
    return pImpl->ElapsedNanoseconds() * 1e-9;
}

double HighResolutionTimer::ElapsedMilliseconds() const
{
    return pImpl->ElapsedNanoseconds() * 1e-6;
}

double HighResolutionTimer::ElapsedMicroseconds() const
{
    return pImpl->ElapsedNanoseconds() * 1e-3;
}

double HighResolutionTimer::ElapsedNanoseconds() const
{
    return pImpl->ElapsedNanoseconds();
}

double HighResolutionTimer::TotalSeconds() const
{
    return pImpl->TotalNanoseconds() * 1e-9;
}

double HighResolutionTimer::TotalMilliseconds() const
{
    return pImpl->TotalNanoseconds() * 1e-6;
}

double HighResolutionTimer::TotalMicroseconds() const
{
    return pImpl->TotalNanoseconds() * 1e-3;
}

double HighResolutionTimer::TotalNanoseconds() const
{
    return pImpl->TotalNanoseconds();
}