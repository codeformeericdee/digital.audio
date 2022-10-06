/*

	Application for an audio driver and buffer manager written by Eric Dee.
	2022

*/

#include "ASIOBuffer.h"
#include "DriverManager.h"
#include <map>

#define NAMEDRIVER "Yamaha Steinberg USB ASIO"

using namespace std;

namespace Playback
{
	typedef struct DriverCache {
		map<int, char*> myDrivers = {};
		map<int, char*>* SendDrivers() { return &this->myDrivers; }
		void AddToMap(int driverType, char* driverName) { this->myDrivers.insert({ driverType, driverName }); };
	};

	Workstation::DriverManager UseDriverCache(DriverCache* drivers)
	{
		return Workstation::DriverManager::GetInstance(drivers->SendDrivers());
	}
}

using namespace Playback;

int main(int argc, char* argv[])
{
	DriverCache myDrivers = DriverCache();
	/* Number of inputand outputs supported by the host. These can be changed. */
	enum { kMaxInputChannels = 32, kMaxOutputChannels = 32 };
	bool openControls = true;

	ASIOChannelInfo asioChannelInfo[kMaxInputChannels + kMaxOutputChannels];

	myDrivers.AddToMap(TYPEASIO, NAMEDRIVER);
	Workstation::DriverManager driverManager = UseDriverCache(&myDrivers);
	driverManager.ChangeToDriver(NAMEDRIVER, openControls);

	/* Buffer instantiation */
	ASIO::ASIOBuffer bufferOne = ASIO::ASIOBuffer();

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