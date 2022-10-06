/*

    Header for audio driver switching written by Eric Dee.
    2022

*/

#pragma once
#include "asio.h"
#include "asiodrivers.h"
#include <cstdio>
#include <map>
#include <memory>
// Points to asiodrivers.cpp
bool loadAsioDriver(char* name);
using namespace std;

typedef struct DriverContainer {
    bool IsInitialized = false;
    ASIODriverInfo AsioDriverInfo;
};

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
        static DriverContainer staticDrivers;
        static map<int, char*>* staticdriverNames; // Set this to delete/shutdown if dereferenced
        static char* staticActiveDriverName;
        DriverManager(map<int, char*>* driverNames);
        bool LoadAsASIODriver(bool openControls = false);
    };
}
