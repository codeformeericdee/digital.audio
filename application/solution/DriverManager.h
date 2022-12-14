/*

    Header for audio driver switching written by Eric Dee.
    2022

*/

#pragma once

#ifndef ASIOLIB
#define ASIOLIB
#include "asio.h"
#include "asiodrivers.h"
#endif

#include <cstdio>
#include <map>
#include <memory>
// Points to asiodrivers.cpp
bool loadAsioDriver(char* name);
using namespace std;

#define TYPEASIO 1
#define TYPEOPENAL 2

namespace Workstation
{
    class DriverManager
    {

    public:
        void operator=(DriverManager const&) = delete;
        static DriverManager& GetInstance(map<int, char*>* driverNames);
        bool ChangeToDriver(char* driverName, bool openControls = false);

    private:
        static struct DriverContainer {
            bool IsInitialized = false;
            ASIODriverInfo AsioDriverInfo;
        } staticDrivers;
        static map<int, char*>* staticDriverNames; // Set this to delete/shutdown if dereferenced
        static char* staticActiveDriverName;
        DriverManager(map<int, char*>* driverNames);
        bool LoadAsASIODriver(bool openControls = false);
    };
}
