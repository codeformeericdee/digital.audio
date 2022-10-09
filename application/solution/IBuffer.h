#pragma once

#include <cstdio>
#include <iostream>
#include <map>

using namespace std;

namespace Buffers
{
    typedef enum BitDepthDataTypes {
        TypeInt32Buffer = 1
    };

    class IBuffer
    {
    public:
        virtual bool AddAmplitudes(void* newAmplitudes) = 0;

    protected:
        static void* y;

        static int
            limitOfInputBuffers, limitOfOutputBuffers,
            hostBitDepth,
            hostSampleRate,
            samplesCompleted;

        static long
            minimumSize, maximumSize, requestedSize,
            numberOfInputChannels, numberOfOutputChannels,
            numberOfInputBuffers, numberOfOutputBuffers;

        int bitDepth32Int;

        bool hasY;

        virtual bool findLimits() = 0;
        virtual bool buildBuffers() = 0;

        bool defineY(bool reset = false);
        bool initialize();

    private:
        map<int, int> bitDepthDataTypes;

        bool setBitDepthDataTypes();

        template<typename dataType>
        bool tryToCallocY();

        template <typename dataType>
        bool resetArray(void* sourceArray, int value, int count);

        virtual bool start() = 0;
    };
}