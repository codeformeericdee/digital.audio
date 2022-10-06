/*

    Header for an ASIO buffer manager written by Eric Dee.
    2022

*/

#pragma once
#include "asio.h"
#include <cstdio>
#include <corecrt_math_defines.h>
#include "math.h"
#include "cstring"

namespace ASIO
{
    class ASIOBuffer
    {

    public:
        ASIOBuffer();

    private:
        long minimumSize, maximumSize, preferredSize, granularity;

        ASIOChannelInfo* channelInfo;
        ASIOCallbacks callBacks;
        static ASIOBufferInfo staticbufferInfo;
        static ASIOTime* bufferSwitchTimeInfo(ASIOTime* timeInfo, long index, ASIOBool processNow);
        static long asioMessages(long selector, long value, void* message, double* opt);
        static void bufferSwitch(long index, ASIOBool processNow);
        static void sampleRateChanged(ASIOSampleRate sRate);
        bool assignCallbacks();
        bool findBufferData();
        bool buildBuffer();
        bool start();
    };
}