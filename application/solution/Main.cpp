/*

	Application for an audio driver and buffer manager written by Eric Dee.
	2022

*/

#include "ASIOBuffer.h"
#include "Playback.h"

#define NAMEDRIVER "Yamaha Steinberg USB ASIO"
#define SAMPLERATEPLAYBACK 44100.0f

using namespace std;

int main(int argc, char* argv[])
{
	Playback::DriverCache myDrivers = Playback::DriverCache();
	/* Number of inputand outputs supported by the host. These can be changed. */
	enum { kMaxInputChannels = 32, kMaxOutputChannels = 32 };
	int channelIOLimits[2] = { kMaxInputChannels, kMaxOutputChannels };
	bool openControls = false;

	ASIOChannelInfo asioChannelInfo[kMaxInputChannels + kMaxOutputChannels];
	ASIOBufferInfo asioBufferInfo[kMaxInputChannels + kMaxOutputChannels];

	myDrivers.AddToMap(TYPEASIO, NAMEDRIVER);
	Workstation::DriverManager driverManager = UseDriverCache(&myDrivers);
	driverManager.ChangeToDriver(NAMEDRIVER, openControls);

	/* Buffer instantiation */
	ASIO::ASIOBuffer bufferOne = ASIO::ASIOBuffer(SAMPLERATEPLAYBACK, &asioChannelInfo[0], &asioBufferInfo[0], channelIOLimits);

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