#include <DirectX12TutorialPCH.h>
#include <HighResolutionTimer.h>

using namespace std::chrono;

#define WINDOWS_CLOCK 0
#define HIGH_RESOLUTION_CLOCK 1
#define STEADY_CLOCK 2

#define USE_CLOCK WINDOWS_CLOCK

#if USE_CLOCK == WINDOWS_CLOCK
class HighResolutionTimerImpl
{
public:
    HighResolutionTimerImpl()
        : elapsedTime(0)
    {
        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&t0);
    }

    void Tick()
    {
        QueryPerformanceCounter(&t1);
        // Compute the value in microseconds (1 second = 1,000,000 microseconds)
        elapsedTime = (t1.QuadPart - t0.QuadPart) * (1000000.0 / frequency.QuadPart);

        t0 = t1;
    }

    double GetElapsedTimeInMicroSeconds()
    {
        return elapsedTime;
    }

private:
    LARGE_INTEGER t0, t1;
    LARGE_INTEGER frequency;
    double elapsedTime;
};
#elif USE_CLOCK == HIGH_RESOLUTION_CLOCK
class HighResolutionTimerImpl
{
public:
    HighResolutionTimerImpl()
        : elapsedTime(0)
    {
        t0 = high_resolution_clock::now();
    }

    void Tick()
    {
        t1 = high_resolution_clock::now();
        duration<double, std::micro> delta = t1 - t0;
        t0 = t1;
        elapsedTime = delta.count();
    }

    double GetElapsedTimeInMicroSeconds()
    {
        return elapsedTime;
    }

private:
    high_resolution_clock::time_point t0, t1;
    double elapsedTime;
};
#elif USE_CLOCK == STEADY_CLOCK
class HighResolutionTimerImpl
{
public:
    HighResolutionTimerImpl()
        : elapsedTime(0)
    {
        t0 = steady_clock::now();
    }

    void Tick()
    {
        t1 = steady_clock::now();
        duration<double, std::micro> delta = t1 - t0;
        t0 = t1;
        elapsedTime = delta.count();
    }

    double GetElapsedTimeInMicroSeconds()
    {
        return elapsedTime;
    }

private:
    steady_clock::time_point t0, t1;
    double elapsedTime;
};
#endif

HighResolutionTimer::HighResolutionTimer(void)
{
    pImpl = new HighResolutionTimerImpl();
}

HighResolutionTimer::~HighResolutionTimer(void)
{
    delete pImpl;
}

void HighResolutionTimer::Tick()
{
    pImpl->Tick();
}

double HighResolutionTimer::ElapsedSeconds() const
{
    return pImpl->GetElapsedTimeInMicroSeconds() * 0.000001;
}

double HighResolutionTimer::ElapsedMilliSeconds() const
{
    return pImpl->GetElapsedTimeInMicroSeconds() * 0.001;
}

double HighResolutionTimer::ElapsedMicroSeconds() const
{
    return pImpl->GetElapsedTimeInMicroSeconds();
}