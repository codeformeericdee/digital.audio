/*

    Code for an output track written by Eric Dee.
    2022

*/

#include "OutputTrack.h"

namespace Tracks
{
    OutputTrack::OutputTrack(double hostSampleRate)
    {
        this->Y = { 0 };
        this->oscillator = new Oscillation::IOscillator(hostSampleRate);
    }
}