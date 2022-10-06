/*

	Code for audio driver switching written by Eric Dee.
	2022

*/

#include "DriverManager.h"

namespace Workstation
{

//Private
	DriverContainer DriverManager::staticDrivers;
	map<int, char*>* DriverManager::staticdriverNames;
	char* DriverManager::staticActiveDriverName;

//Public
    DriverManager& DriverManager::GetInstance(map<int, char*>* driverNames)
    {
		for (const auto& name : (*driverNames))
		{
			switch (name.first)
			{
			case TYPEASIO:
				staticDrivers.AsioDriverInfo = ASIODriverInfo();
				staticDrivers.IsInitialized = true;
				break;
			case TYPEOPENAL:
				break;
			default:
				printf("This cannot run that type of driver.");
				break;
			}
		}
		static DriverManager instance{ driverNames };
        return instance;
    }

    bool DriverManager::ChangeToDriver(char* driverName, bool openControls)
    {
		for (const auto& name : (*staticdriverNames))
		{
			int key = name.first;
			char* archivedName = name.second;

			if (strcmp(archivedName, driverName) == 0)
			{
				switch (key)
				{
				case TYPEASIO:
					staticActiveDriverName = archivedName;
					return this->LoadAsASIODriver(openControls);
				case TYPEOPENAL:
					return false;
				default:
					printf("This cannot run that type of driver.");
					return false;
				}
			}
		}
    }

//Private
    DriverManager::DriverManager(map<int, char*>* driverNames)
    {
		staticdriverNames = driverNames;
    }

	bool DriverManager::LoadAsASIODriver(bool openControls)
	{
		if (loadAsioDriver(staticActiveDriverName))
		{
			staticDrivers.AsioDriverInfo.sysRef = 0; // Find out what this one does
			staticDrivers.AsioDriverInfo.asioVersion = 0; // Find out how to check for the version
			if (ASIOInit(&staticDrivers.AsioDriverInfo) == ASE_OK)
			{
				openControls ? ASIOControlPanel() : NULL;
				printf(
					"---HOST INFO---\n"
					"The driver you are using is: %s\n"
					"The driver is on version number: %d\n"
					"The ASIO version is: %d\n",
					staticDrivers.AsioDriverInfo.name,
					staticDrivers.AsioDriverInfo.driverVersion,
					staticDrivers.AsioDriverInfo.asioVersion
				);
				return true;
			}
			else
			{
				printf(
					"Due to an unknown error, this driver could not be used."
					"Try again or try another driver name."
				);
				return false;
			}
		}
	}
}