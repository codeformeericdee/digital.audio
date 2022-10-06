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
        (ASIOSampleRate sampleRate, ASIOChannelInfo* channelInfo, ASIOBufferInfo* bufferInfo, int channelIOLimits[2]);
        static double NanoSeconds;
        static double Samples;
        static double TimeCodeSamples;

    private:
        int limitOfInputBuffers, limitOfOutputBuffers;

        long minimumSize, maximumSize, preferredSize, granularity,
            numberOfInputChannels, numberOfOutputChannels,
            numberOfInputBuffers, numberOfOutputBuffers,
            inputLatency, outputLatency;

        ASIOChannelInfo* channelInfo;
        ASIOCallbacks callBacks;
        ASIOSampleRate sampleRate;

        static int getSampleRate;
        static int getBufferSize;

        static ASIOBufferInfo* staticBufferInfo;
        static ASIOTime* bufferSwitchTimeInfo(ASIOTime* timeInfo, long index, ASIOBool processNow);
        static ASIOTime* staticTimeInfo;
        static long asioMessage(long selector, long value, void* message, double* opt);
        static void bufferSwitch(long index, ASIOBool processNow);
        static void sampleRateDidChange(ASIOSampleRate sRate);
        
        bool assignCallbacks();
        bool findLimits();
        bool buildBuffers();
        bool start();
        bool setGetters();
    };
}