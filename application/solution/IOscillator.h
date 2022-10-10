#pragma once

#include <corecrt_math_defines.h>

#ifndef STANDARDCPP
    #define STANDARDCPP
    #include <cstdio>
    #include <iostream>
    using namespace std;
#endif // !STANDARDCPP

    #include "math.h"

namespace Oscillation
{
    class IOscillator
    {

    public:
        IOscillator(double hostSampleRate);

        bool ApplyCustomPoints(void* yArray, int amplitude, double frequency);
        bool ApplySineWavePoints(void* yArray, int amplitude, double frequency);

    private:
        double hostSampleRate;

        virtual bool applyCustomPoints(void* yArray, int amplitude, double frequency);
        bool generateSineWave(void* yArray, int amplitude, double frequency);
    };
}