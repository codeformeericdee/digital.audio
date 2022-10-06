/*

	Code for an ASIO buffer manager written by Eric Dee.
	2022

*/

#include "ASIOBuffer.h"

namespace ASIO
{
//Public
	double ASIOBuffer::StaticNanoSeconds;
	double ASIOBuffer::StaticSamples;
	double ASIOBuffer::StaticTimeCodeSamples;

	ASIOBuffer::ASIOBuffer
	(ASIOSampleRate sampleRate, ASIOChannelInfo* channelInfo, ASIOBufferInfo* bufferInfo, int channelIOLimits[2])
	{
		/* Public */
		StaticNanoSeconds = 0; this->StaticSamples = 0; this->StaticTimeCodeSamples = 0;

		/* Private */
		// Limits:
		this->limitOfInputBuffers = channelIOLimits[0];
		this->limitOfOutputBuffers = channelIOLimits[1];

		// Buffer details:
		this->minimumSize = 0; this->maximumSize = 0; this->preferredSize = 0; this->granularity = 0;
		this->inputLatency = 0; this->outputLatency = 0;
		// IO Channels
		this->numberOfInputChannels = 0; this->numberOfOutputChannels = 0;
		// IO Buffers
		this->numberOfInputBuffers = 0; this->numberOfOutputBuffers = 0;

		// Members:
		this->sampleRate = sampleRate;
		this->channelInfo = channelInfo;
		this->staticBufferInfo = bufferInfo;
		
		// Getters:
		this->getSampleRate = this->sampleRate;
		this->getBufferSize = this->preferredSize;
		
		// Initializer:
		this->start();
	}

//Private
	ASIOBufferInfo* ASIOBuffer::staticBufferInfo;
	ASIOTime* ASIOBuffer::staticTimeInfo;
	int ASIOBuffer::getSampleRate;
	int ASIOBuffer::getBufferSize;

	// (Documentation) conversion from 64 bit ASIOSample/ASIOTimeStamp to double float
#if NATIVE_INT64
#define ASIO64toDouble(a)  (a)
#else
	const double twoRaisedTo32 = 4294967296.;
#define ASIO64toDouble(a)  ((a).lo + (a).hi * twoRaisedTo32)
#endif

	ASIOTime* ASIOBuffer::bufferSwitchTimeInfo(ASIOTime* timeInfo, long index, ASIOBool processNow)
	{
		// (Documentation)
		// the actual processing callback.
		// Beware that this is normally in a seperate thread, hence be sure that you take care
		// about thread synchronization. This is omitted here for simplicity.
		static long processedSamples = 0;
		staticTimeInfo = timeInfo;

		// (Documentation)
		// get the time stamp of the buffer, not necessary if no
		// synchronization to other media is required

		/* This converter doesn't seem needed but has been put in to study what it does */
		if (timeInfo->timeInfo.flags & kSystemTimeValid)
			StaticNanoSeconds = ASIO64toDouble(timeInfo->timeInfo.systemTime);
		else
			StaticNanoSeconds = 0;

		if (timeInfo->timeInfo.flags & kSamplePositionValid)
			StaticSamples = ASIO64toDouble(timeInfo->timeInfo.samplePosition);
		else
			StaticSamples = 0;

		if (timeInfo->timeCode.flags & kTcValid)
			StaticTimeCodeSamples = ASIO64toDouble(timeInfo->timeCode.timeCodeSamples);
		else
			StaticTimeCodeSamples = 0;

		/* There is a Windows debug method in the sample to consider implementing here. */

		/* Pull the buffer size for updating */
		long bufferSize = getBufferSize;


		/*
		
			Start here
		
		*/


		/* (Eric)
		A sine wave */
		float* x = new float[getSampleRate];

		for (int i = 0; i < getSampleRate; i++)
		{
			x[i] = (sin(2 * M_PI * 440 * i) / getSampleRate); /* 2^8/2-1=127 */
		}

		memcpy(staticBufferInfo[0].buffers[index], x, getBufferSize);

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
				this->setGetters();

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
							// check wether the driver requires the ASIOOutputReady() optimization
							// (can be used by the driver to reduce output latency by one block)
							if (ASIOOutputReady() == ASE_OK)
							{
								printf("ASIO output set to ready.");
								return true;
								/*asioDriverInfo->postOutput = true;*/
							}
							else
							{
								return false;
								//asioDriverInfo->postOutput = false;
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
		/* Build a buffer specific to this instance that integrates with ASIO
		*  This method is basically just the documentation, but with some class abstraction */
		if (this->findLimits())
		{
			long i;
			ASIOError result;

			ASIOBufferInfo* bufferInfo = staticBufferInfo;

			/* Limits the buffer count to the channel count archived from the construction. */
			if (this->numberOfInputChannels > this->limitOfInputBuffers)
			{
				this->numberOfInputBuffers = this->limitOfInputBuffers;
			}
			else
			{
				this->numberOfInputBuffers = this->numberOfInputChannels;
			}

			for (i = 0; i < this->numberOfInputBuffers; i++, bufferInfo++)
			{
				bufferInfo->isInput = ASIOTrue;
				bufferInfo->channelNum = i;
				bufferInfo->buffers[0] = bufferInfo->buffers[1] = 0; // ?
			}

			result = ASIOCreateBuffers(
				staticBufferInfo, 
				this->numberOfInputBuffers + this->numberOfOutputBuffers, 
				this->preferredSize,
				&this->callBacks
			);

			if (result == ASE_OK)
			{
				/* Gets channel info for items such as bit depth, which are crucial to operation */
				for (i = 0; i < this->numberOfInputBuffers + this->numberOfOutputBuffers; i++)
				{
					this->channelInfo[i].channel = staticBufferInfo[i].channelNum;
					this->channelInfo[i].isInput = staticBufferInfo[i].isInput;
					if (ASIOGetChannelInfo(&this->channelInfo[i]) != ASE_OK)
					{
						printf("The buffer item/channel at index %i could not get set.\n", i);
						break;
					}
				}

				if (result == ASE_OK)
				{
					// (Documentation)
					// get the input and output latencies
					// Latencies often are only valid after ASIOCreateBuffers()
					// (input latency is the age of the first sample in the currently returned audio block)
					// (output latency is the time the first sample in the currently returned audio block requires to get 
					// to the output)
					if (result = ASIOGetLatencies(&this->inputLatency, &this->outputLatency) == ASE_OK)
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
	bool ASIOBuffer::setGetters()
	{
		this->getBufferSize = this->preferredSize;
		this->getSampleRate = this->sampleRate;
		return true;
	}
}