/*

    Header for an ASIO buffer manager written by Eric Dee.
    2022

*/

#pragma once

#ifndef ASIOSYS
#define ASIOSYS
#include "asiosys.h"
#endif

#ifndef ASIOLIB
#define ASIOLIB
#include "asio.h"
#include "asiodrivers.h"
#endif

#include <cstdio>
#include <corecrt_math_defines.h>
#include "math.h"
#include "cstring"

namespace ASIO
{
    class ASIOBuffer
    {

    public:
        ASIOBuffer
        (ASIOSampleRate sampleRate, ASIOChannelInfo* channelInfo, ASIOBufferInfo* bufferInfo, int channelIOLimits[3]);
        static double NanoSeconds;
        static double Samples;
        static double TimeCodeSamples;

    private:
        static int limitOfInputBuffers, limitOfOutputBuffers, hostBitDepth;

        static long 
            minimumSize, maximumSize, preferredSize, granularity,
            numberOfInputChannels, numberOfOutputChannels,
            numberOfInputBuffers, numberOfOutputBuffers,
            inputLatency, outputLatency;

        static ASIOChannelInfo* channelInfo;
        static ASIOBufferInfo* bufferInfo;
        static ASIOSampleRate sampleRate;
        static ASIOTime* timeInfo;

        static ASIOTime* bufferSwitchTimeInfo(ASIOTime* timeInfo, long index, ASIOBool processNow);
        static long asioMessage(long selector, long value, void* message, double* opt);
        static void bufferSwitch(long index, ASIOBool processNow);
        static void sampleRateDidChange(ASIOSampleRate sRate);

        ASIOCallbacks callBacks;

        bool assignCallbacks();
        bool findLimits();
        bool buildBuffers();
        bool start();

        // Data generation (will be abstracted into an oscillator class)
        static void* x;

        static void setSamples(double frequency);
    };
}