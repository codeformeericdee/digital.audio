/*

    Header for an output track written by Eric Dee.
    2022

*/

#pragma once

#include "IOscillator.h"

namespace Tracks
{
    class OutputTrack
    {
    public:
        OutputTrack(double hostSampleRate);

        void* Y;

        Oscillation::IOscillator* oscillator;

        // A apply oscillations function must be created here
    };
}