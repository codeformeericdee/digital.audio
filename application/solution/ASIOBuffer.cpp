/*
	Code for ASIO buffer management written by Eric Dee.
	2022
*/

#include "ASIOBuffer.h"

// (Documentation) conversion from 64 bit ASIOSample/ASIOTimeStamp to double float
#if NATIVE_INT64
#define ASIO64toDouble(a)  (a)
#else
const double twoRaisedTo32 = 4294967296.;
#define ASIO64toDouble(a)  ((a).lo + (a).hi * twoRaisedTo32)
#endif

/* Function for getting the first 24 bits of a 32 bit signed for 24 bit depth. */
signed getBits(signed a, signed b)
{
	// a == lowest
	// b == highest
	signed r = 0;
	for (signed i = a; i <= b; i++)
	{
		r |= 1 << i;
		return r;
	}
}

namespace ASIO
{	
	/* Fields or members */

	// Public
	double 
		ASIOBuffer::NanoSeconds,
		ASIOBuffer::Samples,
		ASIOBuffer::TimeCodeSamples;

	// Private
	int 
		ASIOBuffer::limitOfInputBuffers, 
		ASIOBuffer::limitOfOutputBuffers,
		ASIOBuffer::hostBitDepth,
		ASIOBuffer::samplesCompleted;

	long
		ASIOBuffer::minimumSize, 
		ASIOBuffer::maximumSize, 
		ASIOBuffer::preferredSize, 
		ASIOBuffer::granularity,
		ASIOBuffer::numberOfInputChannels, 
		ASIOBuffer::numberOfOutputChannels,
		ASIOBuffer::numberOfInputBuffers, 
		ASIOBuffer::numberOfOutputBuffers,
		ASIOBuffer::inputLatency, 
		ASIOBuffer::outputLatency;

	ASIOChannelInfo*
		ASIOBuffer::channelInfo;

	ASIOBufferInfo*
		ASIOBuffer::bufferInfo;

	ASIOSampleRate
		ASIOBuffer::sampleRate;

	ASIOTime*
		ASIOBuffer::timeInfo;

	void*
		ASIOBuffer::x;

	/* Methods */
	// Public
	// Constructor
	ASIOBuffer::ASIOBuffer (
		ASIOSampleRate sampleRate, 
		ASIOChannelInfo* channelInfo, 
		ASIOBufferInfo* bufferInfo, 
		int channelIOLimits[3]
	)
	{
		this->NanoSeconds = 0; this->Samples = 0; this->TimeCodeSamples = 0;

		// Channel/buffer limits gets sent as an array:
		this->limitOfInputBuffers = channelIOLimits[0];
		this->limitOfOutputBuffers = channelIOLimits[1];

		// Information for the buffer:
		this->minimumSize = 0; this->maximumSize = 0; this->preferredSize = 0; this->granularity = 0;
		this->inputLatency = 0; this->outputLatency = 0;
		
		// Number of channels from IO gets sent by host
		this->numberOfInputChannels = 0; this->numberOfOutputChannels = 0;
		// Number of buffer channels allowed gets determined by this class
		this->numberOfInputBuffers = 0; this->numberOfOutputBuffers = 0;

		// ASIO specific members:
		this->channelInfo = channelInfo;
		this->bufferInfo = bufferInfo;
		this->sampleRate = sampleRate;
		this->timeInfo = nullptr;
		// Callbacks get set in start.
		
		/* This needs to be handled, but set for testing */
		this->hostBitDepth = channelIOLimits[2];

		// Initializer:
		this->start();
	}

	//Private
	ASIOTime* ASIOBuffer::bufferSwitchTimeInfo(ASIOTime* timeInfo, long index, ASIOBool processNow)
	{
		/*
		
			Stereo:
			The buffer info array is always holding all the buffers.
			The driver decides which channels are in use, and this fills the buffers
			that the driver is requesting based on its available channels

			(This would explain why most DAWs don't dynamically update to new inputs)
			it would be possible, but consume a lot of resources constantly checking
			and reinstating new buffers just to see if something has been plugged in or turned
			on.

			The accepted solution seems to be just inform of all channels, and the interface
			acts as a broadcaster, sending data regardless of whether the input or output
			is physically connected to a trs or xlr.

		*/

		// (Documentation)
		// the actual processing callback.
		// Beware that this is normally in a seperate thread, hence be sure that you take care
		// about thread synchronization. This is omitted here for simplicity.
		static long processedSamples = 0;
		timeInfo = timeInfo;

		// (Documentation)
		// get the time stamp of the buffer, not necessary if no
		// synchronization to other media is required

		/* This converter doesn't seem needed but has been put in to study what it does */
		if (timeInfo->timeInfo.flags & kSystemTimeValid)
			NanoSeconds = ASIO64toDouble(timeInfo->timeInfo.systemTime);
		else
			NanoSeconds = 0;

		if (timeInfo->timeInfo.flags & kSamplePositionValid)
			Samples = ASIO64toDouble(timeInfo->timeInfo.samplePosition);
		else
			Samples = 0;

		if (timeInfo->timeCode.flags & kTcValid)
			TimeCodeSamples = ASIO64toDouble(timeInfo->timeCode.timeCodeSamples);
		else
			TimeCodeSamples = 0;

		/* There is a Windows debug method in the sample to consider implementing here. */

		/*
		
			The actual processing:
		
		*/

		/* Pull the buffer size for updating */
		long bufferSize = preferredSize;

		setSamples(440.0f);

		for (int i = 0; i < (numberOfInputBuffers + numberOfOutputBuffers); i++)
		{
			/* If the buffer in the list of IO buffers is an output buffer */
			if (bufferInfo[i].isInput == false)
			{
				// Process for outputs only
				switch (channelInfo[i].type)
				{
					/* The buffer starts at 8 bits for its depth. */
					/* Using arrays that quantify a higher resolution increases the size by a factor of 8•(Datatype / 8) */

				case ASIOSTInt16LSB:
					memset(bufferInfo[i].buffers[index], 0, bufferSize * 2);
					break;
				case ASIOSTInt24LSB:
					memset(bufferInfo[i].buffers[index], 0, bufferSize * 3);
					break;
				case ASIOSTInt32LSB:
					if (samplesCompleted >= sampleRate)
					{
						break;
					}
					else
					{
						/* Changes the void array indexer to increment by sizeof int temporarily. */
						const int* obj = static_cast<int*>(x);
						memcpy(bufferInfo[i].buffers[index], &obj[samplesCompleted], bufferSize * 4);
					}
					break;
				case ASIOSTFloat32LSB:
					memset(bufferInfo[i].buffers[index], 0, bufferSize * 4);
					break;
				case ASIOSTFloat64LSB:
					memset(bufferInfo[i].buffers[index], 0, bufferSize * 8);
					break;
				}
			}
		}
		
		/* This increments the run count based on buffer size. */
		samplesCompleted += bufferSize;

		// (Documentation)
		// finally if the driver supports the ASIOOutputReady() optimization, do it here, all data are in place
		if (true)
		{
			ASIOOutputReady();
		}

		return 0L;
	}

	long ASIOBuffer::asioMessage(long selector, long value, void* message, double* opt)
	{
		/* This function is taken directly from the documentation. */
		
		// currently the parameters "value", "message" and "opt" are not used.
		long ret = 0;
		switch (selector)
		{
		case kAsioSelectorSupported:
			if (value == kAsioResetRequest
				|| value == kAsioEngineVersion
				|| value == kAsioResyncRequest
				|| value == kAsioLatenciesChanged
				// the following three were added for ASIO 2.0, you don't necessarily have to support them
				|| value == kAsioSupportsTimeInfo
				|| value == kAsioSupportsTimeCode
				|| value == kAsioSupportsInputMonitor)
				ret = 1L;
			break;
		case kAsioResetRequest:
			// defer the task and perform the reset of the driver during the next "safe" situation
			// You cannot reset the driver right now, as this code is called from the driver.
			// Reset the driver is done by completely destruct is. I.e. ASIOStop(), ASIODisposeBuffers(), Destruction
			// Afterwards you initialize the driver again.
			/* ASIODRIVERINFO.stopped;  // In this sample the processing will just stop */
			ret = 1L;
			break;
		case kAsioResyncRequest:
			// This informs the application, that the driver encountered some non fatal data loss.
			// It is used for synchronization purposes of different media.
			// Added mainly to work around the Win16Mutex problems in Windows 95/98 with the
			// Windows Multimedia system, which could loose data because the Mutex was hold too long
			// by another thread.
			// However a driver can issue it in other situations, too.
			ret = 1L;
			break;
		case kAsioLatenciesChanged:
			// This will inform the host application that the drivers were latencies changed.
			// Beware, it this does not mean that the buffer sizes have changed!
			// You might need to update internal delay data.
			ret = 1L;
			break;
		case kAsioEngineVersion:
			// return the supported ASIO version of the host application
			// If a host applications does not implement this selector, ASIO 1.0 is assumed
			// by the driver
			ret = 2L;
			break;
		case kAsioSupportsTimeInfo:
			// informs the driver wether the asioCallbacks.bufferSwitchTimeInfo() callback
			// is supported.
			// For compatibility with ASIO 1.0 drivers the host application should always support
			// the "old" bufferSwitch method, too.
			ret = 1;
			break;
		case kAsioSupportsTimeCode:
			// informs the driver wether application is interested in time code info.
			// If an application does not need to know about time code, the driver has less work
			// to do.
			ret = 0;
			break;
		}
		return ret;
	}

	void ASIOBuffer::bufferSwitch(long index, ASIOBool processNow)
	{
		throw "Not implemented";
	}

	void ASIOBuffer::sampleRateDidChange(ASIOSampleRate sRate)
	{
		throw "Not implemented";
	}

	bool ASIOBuffer::assignCallbacks()
	{
		/* Assign event callback handling that integrates with ASIOCallbacks */
		this->callBacks.asioMessage = &asioMessage;
		this->callBacks.bufferSwitchTimeInfo = &bufferSwitchTimeInfo;
		this->callBacks.bufferSwitch = &bufferSwitch;
		this->callBacks.sampleRateDidChange = &sampleRateDidChange;
		return true;
	}

	bool ASIOBuffer::findLimits()
	{
		printf("---ASIO BUFFER INFO---\n");
		if (ASIOGetChannels(&this->numberOfInputChannels, &this->numberOfOutputChannels) == ASE_OK)
		{ 
			printf(
				"The number of input channels is: %d | The number of output channels is: %d.\n",
				this->numberOfInputChannels, this->numberOfOutputChannels
			);
			/* Determine buffer size(Should be called before doing any changes - hence using a struct to store this
			data) */
			if (ASIOGetBufferSize(
				&this->minimumSize, &this->maximumSize, &this->preferredSize, &this->granularity) == ASE_OK)
			{
				printf("The buffer info has been found.\n");
				printf(
					"The buffer requirements in bytes are [ minimum: %d, maximum: %d, preferred: %d, granularity: %d ]\n",
					this->minimumSize, this->maximumSize, this->preferredSize, this->granularity
				);
				ASIOSampleRate testRate;
				if (ASIOGetSampleRate(&testRate) == ASE_OK)
				{
					printf("The sample rate is %f.\n", this->sampleRate);
					if (testRate != this->sampleRate)
					{
						if (ASIOSetSampleRate(this->sampleRate) == ASE_OK)
						{
							printf(
								"The buffer sample rate was changed."
								"---END OF ASIO BUFFER INFO---\n"
							);
							// check whether the driver requires the ASIOOutputReady() optimization
							// (can be used by the driver to reduce output latency by one block)
							if (ASIOOutputReady() == ASE_OK)
							{
								printf("ASIO output set to ready.");
								//this->postOutput = true;
								return true;
							}
							else
							{
								//this->postOutput = false;
								return false;
							}
						}
						else
						{
							printf(
								"The buffer sample rate was not changed.\n"
								"The buffer sample rate may not match the host rate.\n"
								"---END OF ASIO BUFFER INFO---\n"
							);
							return false;
						}
					}
					else
					{
						printf(
							"The buffer sample rate already matched the default rate.\n"
						);
						if (ASIOOutputReady() == ASE_OK)
						{
							printf(
								"The output is set to ready.\n"
								"---END OF ASIO BUFFER INFO---\n"
							);
							return true;
							/*asioDriverInfo->postOutput = true;*/
						}
						else
						{
							printf("---END OF ASIO BUFFER INFO---\n");
							return false;
						}
					}
				}
				else
				{
					printf(
						"The sample rate has not been found.\n"
						"---END OF ASIO BUFFER INFO---\n"
					);
					return false;
				}
			}
			else
			{
				printf(
					"The buffer info has not been found.\n"
					"---END OF ASIO BUFFER INFO---\n"
				);
				return false;
			}
		}
		else
		{
			printf(
				"The channel info has not been found.\n"
				"---END OF ASIO BUFFER INFO---\n"
			);
			return false;
		}
	}

	bool ASIOBuffer::buildBuffers()
	{
		/*
		
			Stereo:
			The channel count should always return 2 from the driver if two speakers.
			Therefore this can be expected to generate the need for two arrays to be written
			regardless of whether the data is mono or not, both speakers will need to be played from.

		*/

		/* Build a buffer specific to this instance that integrates with ASIO
		   This method is basically just the documentation, but with some class abstraction */
		if (this->findLimits())
		{
			long i;
			ASIOError result;

			/* Local copy allows non invasive adjusting */
			ASIOBufferInfo* localBufferInfo = bufferInfo;

			/* Limits the buffer count to the channel count archived from the construction. */

			// Input
			if (this->numberOfInputChannels > this->limitOfInputBuffers)
			{
				this->numberOfInputBuffers = this->limitOfInputBuffers;
			}
			else
			{
				this->numberOfInputBuffers = this->numberOfInputChannels;
			}
			for (i = 0; i < this->numberOfInputBuffers; i++, localBufferInfo++)
			{
				localBufferInfo->isInput = ASIOTrue;
				localBufferInfo->channelNum = i;
				localBufferInfo->buffers[0] = localBufferInfo->buffers[1] = 0; // ?
			}

			// Output
			if (this->numberOfOutputChannels > this->limitOfOutputBuffers)
			{
				this->numberOfOutputBuffers = this->limitOfOutputBuffers;
			}
			else
			{
				this->numberOfOutputBuffers = this->numberOfOutputChannels;
			}
			for (i = 0; i < this->numberOfOutputBuffers; i++, localBufferInfo++)
			{
				localBufferInfo->isInput = ASIOFalse;
				localBufferInfo->channelNum = i;
				localBufferInfo->buffers[0] = localBufferInfo->buffers[1] = 0; // ?
			}

			// Instantiate the buffer off of this class from heap
			result = ASIOCreateBuffers(
				this->bufferInfo,
				this->numberOfInputBuffers + this->numberOfOutputBuffers,
				this->preferredSize,
				&this->callBacks
			);

			if (result == ASE_OK)
			{
				/* Gets channel info for items such as bit depth, which are crucial to operation */
				for (i = 0; i < (this->numberOfInputBuffers + this->numberOfOutputBuffers); i++)
				{
					this->channelInfo[i].channel = bufferInfo[i].channelNum;
					this->channelInfo[i].isInput = bufferInfo[i].isInput;

					result = ASIOGetChannelInfo(&this->channelInfo[i]);
					if (result != ASE_OK)
					{
						printf("The buffer item/channel at index %i could not get set.\n", i);
						break;
					}

					// Host settings:
					this->channelInfo[i].type = this->hostBitDepth;
				}

				if (result == ASE_OK)
				{
					// (Documentation)
					// get the input and output latencies
					// Latencies often are only valid after ASIOCreateBuffers()
					// (input latency is the age of the first sample in the currently returned audio block)
					// (output latency is the time the first sample in the currently returned audio block requires to get 
					// to the output)
					result = ASIOGetLatencies(&this->inputLatency, &this->outputLatency);
					if (result == ASE_OK)
					{
						printf(
							"The latencies are %d for input, and %d for output.\n", 
							this->inputLatency, this->outputLatency
						);
						return true;
					}
					else
					{
						return false;
					}
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	bool ASIOBuffer::start()
	{
		return this->assignCallbacks() ? this->buildBuffers() : false;
	}

	void ASIOBuffer::setSamples(double frequency)
	{
		x = new int[sampleRate];
		double sample = 0;
		int* obj = static_cast<int*>(x);
		for (int i = 0; i < sampleRate; i++)
		{
			sample = (sin(2 * 3.14 * frequency * i / sampleRate)) / sampleRate;
			obj[i] = (int)(sample * 2000000000000);
		}
	}
}