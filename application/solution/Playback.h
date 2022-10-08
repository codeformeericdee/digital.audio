/*

    Header for audio playback requests written by Eric Dee.
    2022

*/

#include <map>

#include "DriverManager.h"

namespace Playback
{
    typedef struct DriverCache {
        map<int, char*> myDrivers = {};
        map<int, char*>* SendDrivers();
        void AddToMap(int driverType, char* driverName);
    };
    Workstation::DriverManager UseDriverCache(DriverCache* drivers);
}