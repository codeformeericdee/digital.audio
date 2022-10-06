/*

	Application for an audio driver and buffer manager written by Eric Dee.
	2022

*/

#include "ASIOBuffer.h"
#include "DriverManager.h"
#include <map>

#define NAMEDRIVER "Yamaha Steinberg USB ASIO"

using namespace std;

struct DriverCache {
	map<int, char*> myDrivers = {};
	map<int, char*>* SendDrivers() { return &this->myDrivers; }
	void AddToMap(int driverType, char* driverName) { this->myDrivers.insert({ driverType, driverName }); };
} myDrivers;

Workstation::DriverManager UseMyDrivers()
{
	return Workstation::DriverManager::GetInstance(myDrivers.SendDrivers());
}

int main(int argc, char* argv[])
{
	/* Number of inputand outputs supported by the host. These can be changed. */
	enum { kMaxInputChannels = 32, kMaxOutputChannels = 32 };
	bool openControls = true;

	ASIOChannelInfo asioChannelInfo[kMaxInputChannels + kMaxOutputChannels];
	ASIODriverInfo asioDriverInfo;

	myDrivers.AddToMap(TYPEASIO, NAMEDRIVER);
	Workstation::DriverManager driverManager = UseMyDrivers();
	driverManager.ChangeToDriver(NAMEDRIVER, openControls);

	/* Buffer instantiation */
	ASIO::ASIOBuffer bufferOne = ASIO::ASIOBuffer(&asioDriverInfo, asioChannelInfo);

	// ASIOGetChannelInfo(channelInfos);

	// Start ASIO
	if (ASIOStart() == ASE_OK)
	{
		fprintf(stdout, "---STATUS---\nDriver did start.\n\n");

		Sleep(1000);

		ASIOStop();
		ASIODisposeBuffers();
		ASIOExit();
	}

	return 0;
}