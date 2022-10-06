/*

	Application for an audio driver and buffer manager written by Eric Dee.
	2022

*/

#include "ASIOBuffer.h"
#include "DriverManager.h"

#define NAMEDRIVER "Yamaha Steinberg USB ASIO"

int main(int argc, char* argv[])
{
	/* Number of inputand outputs supported by the host.These can be changed. */
	enum { kMaxInputChannels = 32, kMaxOutputChannels = 32 };
	bool openControls = true;

	ASIOChannelInfo asioChannelInfo[kMaxInputChannels + kMaxOutputChannels];
	ASIODriverInfo asioDriverInfo;

	DriverContainer drivers = {
		&asioDriverInfo
	};

	Workstation::DriverManager driverManager = Workstation::DriverManager::GetInstance(NAMEDRIVER, &drivers);
	driverManager.ChangeToDriver(TYPEASIO, openControls);

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