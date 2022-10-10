/*

	Application for an audio driver and buffer manager written by Eric Dee.
	2022

*/

#include "ASIOBuffer.h"
#include "OutputTrack.h"
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

	Tracks::OutputTrack outputOne = Tracks::OutputTrack(SAMPLERATEPLAYBACK);
	outputOne.Y = new int[SAMPLERATEPLAYBACK];
	outputOne.oscillator->ApplySineWavePoints(outputOne.Y, maxAmplitude, 440.0f);
	bufferOne.AddAmplitudes(outputOne.Y);

	/*
		Please note that if you come from an OOP background,
		the first thought is usually to privatize the track class...
		due to the nature of it being constantly changed, it must remain
		mostly dynamic to the host. Unless you have experience producing in a DAW
		such as Ableton, Reaper, ProTools, Bitwig, or other high level workstations
		then this might not make sense to you.

		While I agree completely with restrictive methods to ensure best application,
		many of my tests will be with the host in mind, and might not become encapsulated
		until later stages.

		This is intended to operate under the very object oriented runtime API that I
		have been writing in C#, which can be found here: https://github.com/codeformeericdee/runtime.api

		Once this application is fully functional as a wave writing, and wave playing device 
		with circular buffers, it will be worked into OpenGL for the ImGUI UX interface, and 
		the classes will be slowly broken down into their most optimal states as I can find. 
		The runtime API will be implemented once I have deemed that it is needed.

		Additionally, I have already experimented with contextual scope, and decided not to
		use a context class to store and operate on data. Each track is expected to carry its 
		weight, and basically contain only oscillators or synthesizers/plugins, and the host 
		should end up with a simple playback class that has administrative rights from the host,
		and holds a vector of track runtimes to be called. It is similar to a context, but is 
		not expected to use any injection. As each item is planned to be reliant on its 
		constructor. If the scope changes, buffers can be rebuilt, as each track should use
		strictly void pointers which the buffer is expected to interpolate based on the host
		settings/requests/flags.
	*/

				// Make a utility class for the arrays

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