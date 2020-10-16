#pragma once

#include <memory> // for std:unique_ptr

class HighResolutionTimer
{
public:
    HighResolutionTimer();
    ~HighResolutionTimer();

    /**
     * Tick the high resolution timer.
     */
    void Tick();

    /**
     * Reset the elapsed and total time.
     */
    void Reset();
    
    /**
     * Get the elapsed time between ticks.
     */
    double ElapsedSeconds() const;
    double ElapsedMilliseconds() const;
    double ElapsedMicroseconds() const;
    double ElapsedNanoseconds() const;

    /**
     * Get the total time since the timer was started (or reset).
     */
    double TotalSeconds() const;
    double TotalMilliseconds() const;
    double TotalMicroseconds() const;
    double TotalNanoseconds() const;

private:
    class impl;
    std::unique_ptr<impl> pImpl;
};
