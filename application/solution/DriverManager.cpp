/*

	Code for audio driver switching written by Eric Dee.
	2022

*/

#include "DriverManager.h"

namespace Workstation
{

//Public
    DriverManager& DriverManager::GetInstance(char* driverName, DriverContainer* drivers)
    {
		static DriverManager instance{driverName, drivers};
        return instance;
    }

    bool DriverManager::ChangeToDriver(int type, bool openControls)
    {
		switch (type)
		{
		case 1:
			return this->LoadASIODriver(openControls);
		case 2:
			return false;
		default:
			printf("A driver type must be specified from 1(ASIO) or 2(OPENAL)");
			return false;
		}
    }

//Private
	char* DriverManager::staticDriverName;
	DriverContainer* DriverManager::staticDrivers;

    DriverManager::DriverManager(char* driverName, DriverContainer* drivers)
    {
		staticDriverName = driverName;
		staticDrivers = drivers;
    }

	bool DriverManager::LoadASIODriver(bool openControls)
	{
		if (loadAsioDriver(staticDriverName))
		{
			staticDrivers->asioDriverInfo->sysRef = 0; // Find out what this one does
			staticDrivers->asioDriverInfo->asioVersion = 0; // Find out how to check for the version
			if (ASIOInit(staticDrivers->asioDriverInfo) == ASE_OK)
			{
				openControls ? ASIOControlPanel() : NULL;
				printf(
					"---HOST INFO---\n"
					"The driver you are using is: %s\n"
					"The driver is on version number: %d\n"
					"The ASIO version is: %d\n",
					staticDrivers->asioDriverInfo->name,
					staticDrivers->asioDriverInfo->driverVersion,
					staticDrivers->asioDriverInfo->asioVersion);
				return true;
			}
			else
			{
				return false;
			}
		}
	}
}