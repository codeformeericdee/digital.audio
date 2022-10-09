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
#include <vector>

using namespace std;

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

        bool AddAmplitudes(void* newAmplitudes);

    private:
        static int limitOfInputBuffers, limitOfOutputBuffers, hostBitDepth, samplesCompleted;

        static long 
            minimumSize, maximumSize, preferredSize, granularity,
            numberOfInputChannels, numberOfOutputChannels,
            numberOfInputBuffers, numberOfOutputBuffers,
            inputLatency, outputLatency;

        static void* y;

        static ASIOChannelInfo* channelInfo;
        static ASIOBufferInfo* bufferInfo;
        static ASIOSampleRate sampleRate;
        static ASIOTime* timeInfo;

        static ASIOTime* bufferSwitchTimeInfo(ASIOTime* timeInfo, long index, ASIOBool processNow);
        static long asioMessage(long selector, long value, void* message, double* opt);
        static void bufferSwitch(long index, ASIOBool processNow);
        static void sampleRateDidChange(ASIOSampleRate sRate);
        static bool addAmplitudes(void* newAmplitudes);

        ASIOCallbacks callBacks;

        template <typename dataType>
        bool resetArray(void* sourceArray, int value, int count);

        bool assignCallbacks();
        bool findLimits();
        bool buildBuffers();
        bool start();
    };
}