#pragma once

#include "DirectX12TutorialDefines.h"

class HighResolutionTimerImpl;

class DX12TL_DLL HighResolutionTimer
{
public:
    HighResolutionTimer( void );
    ~HighResolutionTimer( void );

    // "Tick" the timer to compute the amount of time since the last it was ticked (or since the timer was created).
    void Tick();

    double ElapsedSeconds() const;
    double ElapsedMilliSeconds() const;
    double ElapsedMicroSeconds() const;

private:
    HighResolutionTimerImpl* pImpl;
};
