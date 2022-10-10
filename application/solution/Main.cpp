/*

	Application for an audio driver and buffer manager written by Eric Dee.
	2022

*/

#include "ASIOBuffer.h"
#include "IOscillator.h"
#include "Playback.h"

#define NAMEDRIVER "Yamaha Steinberg USB ASIO"
#define SAMPLERATEPLAYBACK 44100.0f

#ifndef MATHCPP
#define MATHCPP
#include "math.h"
#endif // !MATHCPP

using namespace std;

int main(int argc, char* argv[])
{
	/* Determine drivers */
	Playback::DriverCache myDrivers = Playback::DriverCache();
	
	/* Assign host limitations */
	enum { kMaxInputChannels = 32, kMaxOutputChannels = 32, hostBitDepth = 32 };
	int maxAmplitude = pow(2, (unsigned long long)hostBitDepth) / 2 - 1;
	int channelIOLimits[3] = { kMaxInputChannels, kMaxOutputChannels, ASIOSTInt32LSB };
	
	/* Open the driver? */
	bool openControls = false;

	/* Add available drivers, and use one */
	myDrivers.AddToMap(TYPEASIO, NAMEDRIVER);
	Workstation::DriverManager driverManager = UseDriverCache(&myDrivers);
	driverManager.ChangeToDriver(NAMEDRIVER, openControls);

	/* Instantiate the buffer. This should be changed to a factory method based on the selected driver */
	ASIO::ASIOBuffer bufferOne = ASIO::ASIOBuffer(SAMPLERATEPLAYBACK, channelIOLimits);

	/* Oscillators(later to be tacks containing synthesizers or samplers) */
	Oscillation::IOscillator oscillatorOne = Oscillation::IOscillator(SAMPLERATEPLAYBACK);

	/* Always a factor of bit depth. Typically signed hence divided by two, minus 1 for the sign holder */
	void* trackOneY = new int[SAMPLERATEPLAYBACK];
	void* trackTwoY = new int[SAMPLERATEPLAYBACK];

	oscillatorOne.ApplySineWavePoints(trackOneY, maxAmplitude/18, 440.0f);
	bufferOne.AddAmplitudes(trackOneY);

	oscillatorOne.ApplySineWavePoints(trackTwoY, maxAmplitude/9, 810.0f);
	bufferOne.AddAmplitudes(trackTwoY);

	// Start ASIO (while i < seconds)
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