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
	enum { kMaxInputChannels = 32, kMaxOutputChannels = 32, hostBitDepth = 32 };
	int channelIOLimits[3] = { kMaxInputChannels, kMaxOutputChannels, ASIOSTInt32LSB };
	bool openControls = false;

	myDrivers.AddToMap(TYPEASIO, NAMEDRIVER);
	Workstation::DriverManager driverManager = UseDriverCache(&myDrivers);
	driverManager.ChangeToDriver(NAMEDRIVER, openControls);

	ASIO::ASIOBuffer bufferOne = ASIO::ASIOBuffer(SAMPLERATEPLAYBACK, channelIOLimits);

	double sample = 0;
	int* obj = new int[SAMPLERATEPLAYBACK];
	int maxAmplitude = pow(2, sizeof(int) * 8) / 2 - 1;
	for (int i = 0; i < SAMPLERATEPLAYBACK; i++)
	{
		sample = (sin((2 * M_PI * 440.0f) * (i / SAMPLERATEPLAYBACK)));
		obj[i] = (int)(sample * maxAmplitude / 18);
	}

	bufferOne.AddAmplitudes(obj);

	for (int i = 0; i < SAMPLERATEPLAYBACK; i++)
	{
		sample = (sin((2 * M_PI * 810.0f) * (i / SAMPLERATEPLAYBACK)));
		obj[i] = (int)(sample * maxAmplitude / 9);
	}

	bufferOne.AddAmplitudes(obj);

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