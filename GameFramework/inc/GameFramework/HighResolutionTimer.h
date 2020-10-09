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
     * Reset the timer.
     */
    void Reset();
    
    /**
     * Get the elapsed time between ticks.
     */
    double ElapsedSeconds() const;
    double ElapsedMilliseconds() const;
    double ElapsedMicroseconds() const;

    /**
     * Get the total time since the timer was started (or reset).
     */
    double TotalSeconds() const;
    double TotalMilliseconds() const;
    double TotalMicroseconds() const;

private:
    class impl;
    std::unique_ptr<impl> pImpl;
};
