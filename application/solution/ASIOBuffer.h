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

#include <corecrt_math_defines.h>
#include "math.h"
#include "cstring"
#include <vector>

#ifndef PLAYBACKBUFFERS
#define PLAYBACKBUFFERS
#include "IBuffer.h"
#endif // !PLAYBACKBUFFERS

namespace ASIO
{
    class ASIOBuffer : Buffers::IBuffer
    {

    public:
        ASIOBuffer
        (ASIOSampleRate sampleRate, int channelIOLimits[3]);
        static double NanoSeconds;
        static double Samples;
        static double TimeCodeSamples;

        bool AddAmplitudes(void* newAmplitudes) override;

    private:
        static long
            granularity,
            inputLatency, outputLatency;

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

        bool findLimits() override;
        bool buildBuffers() override;
        bool start() override;

        bool assignCallbacks();
        ASIOSampleRate wrapSampleRate(ASIOSampleRate newRate = NULL);
    };
}