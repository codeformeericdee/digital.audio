/*

    Header for audio driver switching written by Eric Dee.
    2022

*/

#pragma once
#include "asio.h"
#include "asiodrivers.h"
#include <cstdio>
// Points to asiodrivers.cpp
bool loadAsioDriver(char* name);

typedef struct DriverContainer {
    ASIODriverInfo* asioDriverInfo;
};

#define TYPEASIO 1
#define TYPEOPENAL 2

namespace Workstation
{
    class DriverManager
    {

    public:
        void operator=(DriverManager const&) = delete;
        static DriverManager& GetInstance(char* driverName, DriverContainer* drivers);
        bool ChangeToDriver(int type, bool openControls = false);

    private:
        static DriverContainer* staticDrivers;
        static char* staticDriverName;
        DriverManager(char* driverName, DriverContainer* drivers);
        bool LoadASIODriver(bool openControls = false);
    };
}
