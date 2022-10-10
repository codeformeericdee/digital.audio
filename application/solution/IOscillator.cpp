/*

	Code for an oscillator written by Eric Dee.
	2022

*/

#include "IOscillator.h"

namespace Oscillation
{
	IOscillator::IOscillator(double hostSampleRate)
	{
		this->hostSampleRate = hostSampleRate;
	}

	bool IOscillator::ApplyCustomPoints(void* yArray, int amplitude, double frequency)
	{
		return this->applyCustomPoints(yArray, amplitude, frequency);
	}

	bool IOscillator::ApplySineWavePoints(void* yArray, int amplitude, double frequency)
	{
		return this->generateSineWave(yArray, amplitude, frequency);
	}

	bool IOscillator::applyCustomPoints(void* yArray, int amplitude, double frequency)
	{
		throw "Not implemented";
	}

	bool IOscillator::generateSineWave(void* yArray, int amplitude, double frequency)
	{
		try
		{
			double sample = 0;
			int* obj = static_cast<int*>(yArray);
			for (int i = 0; i < this->hostSampleRate; i++)
			{
				sample = sin((2 * M_PI * frequency) * (i / this->hostSampleRate));
				obj[i] = (int)(sample * amplitude);
			}
		}
		catch (exception ex)
		{
			printf("The oscillator could not generate a sine wave due to this error:\n%s\n", ex.what());
			return false;
		}
	}
}