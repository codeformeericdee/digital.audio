/*

	Code for an ASIO buffer manager written by Eric Dee.
	2022

*/

#include "ASIOBuffer.h"

namespace ASIO
{
//Public
	ASIOBuffer::ASIOBuffer()
	{
		this->minimumSize = 0;
		this->maximumSize = 0;
		this->preferredSize = 0;
		this->granularity = 0;
		this->start();
	}

//Private
	ASIOBufferInfo ASIOBuffer::staticbufferInfo;

	ASIOTime* ASIOBuffer::bufferSwitchTimeInfo(ASIOTime* timeInfo, long index, ASIOBool processNow)
	{
		// (Documentation)
		// the actual processing callback.
		// Beware that this is normally in a seperate thread, hence be sure that you take care
		// about thread synchronization. This is omitted here for simplicity.

		/* (Eric)
		A sine wave */
		unsigned char x[1024];
		for (int i = 0; i < 1024; i++)
		{
			x[i] = sin(2 * M_PI * (char)(1024 / 512) * i / 1024) * 127; /* 2^8/2-1=127 */
		}

		memcpy(staticbufferInfo.buffers[index], x, 1024);

		// (Documentation)
		// finally if the driver supports the ASIOOutputReady() optimization, do it here, all data are in place
		if (true)
		{
			ASIOOutputReady();
		}

		return 0L;
	}

	long ASIOBuffer::asioMessages(long selector, long value, void* message, double* opt)
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

	void ASIOBuffer::sampleRateChanged(ASIOSampleRate sRate)
	{
		throw "Not implemented";
	}

	bool ASIOBuffer::assignCallbacks()
	{
		/* Assign event callback handling that integrates with ASIOCallbacks */
		this->callBacks.asioMessage = &asioMessages;
		this->callBacks.bufferSwitchTimeInfo = &bufferSwitchTimeInfo;
		this->callBacks.bufferSwitch = &bufferSwitch;
		this->callBacks.sampleRateDidChange = &sampleRateChanged;
		return true;
	}

	bool ASIOBuffer::findBufferData()
	{
		/* Determine buffer size(Should be called before doing any changes - hence using a struct to store this 
		data) */
		if (ASIOGetBufferSize(
			&this->minimumSize, &this->maximumSize, &this->preferredSize, &this->granularity) == ASE_OK)
		{
			printf("The buffer info has been found.\n");
			printf("The buffer requirements in bytes are [ minimum: %d, maximum: %d, preferred: %d, granularity: %d ]\n",
				this->minimumSize, this->maximumSize, this->preferredSize, this->granularity);
			return true;
		}
		else
		{
			printf("The buffer info has not been found.\n");
			return false;
		}
	}

	bool ASIOBuffer::buildBuffer()
	{
		/* Build a buffer specific to this instance that integrates with ASIO */
		staticbufferInfo.isInput = ASIOFalse;
		staticbufferInfo.channelNum = 0;
		ASIOCreateBuffers(&staticbufferInfo, 1, 256, &this->callBacks);
		return this->findBufferData();
	}

	bool ASIOBuffer::start()
	{
		return this->assignCallbacks() ? this->buildBuffer() : false;
	}
}