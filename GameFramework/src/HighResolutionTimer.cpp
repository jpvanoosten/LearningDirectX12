#include <GameFrameworkPCH.h>

#include <HighResolutionTimer.h>

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
    , elapsedTime( 0.0 )
    , totalTime( 0.0 )
    {
        ::QueryPerformanceFrequency( &frequency );
        ::QueryPerformanceCounter( &t0 );
    }

    void Tick()
    {
        ::QueryPerformanceCounter( &t1 );
        // Compute the value in microseconds (1 second = 1,000,000 microseconds)
        elapsedTime =
            ( t1.QuadPart - t0.QuadPart ) * ( 1000000.0 / frequency.QuadPart );

        totalTime += elapsedTime;

        t0 = t1;
    }

    void Reset()
    {
        ::QueryPerformanceCounter( &t0 );
        elapsedTime = 0.0;
        totalTime   = 0.0;
    }

    double ElapsedMicroseconds() const
    {
        return elapsedTime;
    }

    double TotalMicroseconds() const
    {
        return totalTime;
    }

private:
    LARGE_INTEGER t0, t1;
    LARGE_INTEGER frequency;
    double        elapsedTime;
    double        totalTime;
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
        t1                                 = high_resolution_clock::now();
        duration<double, std::micro> delta = t1 - t0;
        t0                                 = t1;
        elapsedTime                        = delta.count();
        totalTime += elapsedTime;
    }

    void Reset()
    {
        t0          = high_resolution_clock::now();
        elapsedTime = 0.0;
        totalTime   = 0.0;
    }

    double ElapsedMicroseconds() const
    {
        return elapsedTime;
    }

    double TotalMicroseconds() const
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
        t1                                 = steady_clock::now();
        duration<double, std::micro> delta = t1 - t0;
        t0                                 = t1;
        elapsedTime                        = delta.count();
    }

    void Reset()
    {
        t0          = steady_clock::now();
        elapsedTime = 0.0;
        totalTime   = 0.0;
    }

    double ElapsedMicroseconds() const
    {
        return elapsedTime;
    }

    double TotalMicroseconds() const
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
    return pImpl->ElapsedMicroseconds() * 0.000001;
}

double HighResolutionTimer::ElapsedMilliseconds() const
{
    return pImpl->ElapsedMicroseconds() * 0.001;
}

double HighResolutionTimer::ElapsedMicroseconds() const
{
    return pImpl->ElapsedMicroseconds();
}

double HighResolutionTimer::TotalSeconds() const
{
    return pImpl->TotalMicroseconds() * 0.000001;
}

double HighResolutionTimer::TotalMilliseconds() const
{
    return pImpl->TotalMicroseconds() * 0.001;
}

double HighResolutionTimer::TotalMicroseconds() const
{
    return pImpl->TotalMicroseconds();
}