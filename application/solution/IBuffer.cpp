/*

	Code for a versatile buffer interface written by Eric Dee.
	2022

*/

#include "IBuffer.h"

namespace Buffers
{
    void* IBuffer::y;

    int
        IBuffer::limitOfInputBuffers,
        IBuffer::limitOfOutputBuffers,
        IBuffer::hostBitDepth,
        IBuffer::hostSampleRate,
        IBuffer::samplesCompleted;

    long
        IBuffer::minimumSize,
        IBuffer::maximumSize,
        IBuffer::requestedSize,
        IBuffer::numberOfInputChannels,
        IBuffer::numberOfOutputChannels,
        IBuffer::numberOfInputBuffers,
        IBuffer::numberOfOutputBuffers;

	bool IBuffer::AddAmplitudes(void* newAmplitudes)
	{
		return this->addAmplitudes(newAmplitudes);
	}

/* Protected methods */
	bool IBuffer::defineY(bool reset)
	{
		/* 
			Array definition method for calloc or manaul array resetting based on host needs.
			This will iterate through maps defined during construction to determine which array size is needed.
		*/
		for (auto const& bitDepthCase : this->bitDepthDataTypes)
		{
			if (bitDepthCase.second == this->hostBitDepth)
			{
				switch (bitDepthCase.first)
				{
				case TypeInt32Buffer:
					try
					{
						return reset ?
							this->hasY = this->resetArray<int>(this->y, 0, this->hostSampleRate) 
							: this->hasY = this->tryToCallocY<int>();
					}
					catch (exception ex)
					{
						throw exception(("The buffer could not generate a 32 bit depth array with this error:\n%s\n", ex.what()));
					}
				default:
					throw exception("This datatype is not implemented.");
					return false;
				}
			}
			else
			{
				this->hasY = false;
			}
		}
	}

	bool IBuffer::initialize()
	{
		return this->setBitDepthDataTypes() ? this->start() : false;
	}

/* Private methods */
	bool IBuffer::setBitDepthDataTypes()
	{
		this->bitDepthDataTypes.insert({ 1, this->bitDepth32Int });
		return true;
	}

	bool IBuffer::addAmplitudes(void* newAmplitudes)
	{
		if (this->hasY)
		{
			for (auto const& bitDepthCase : this->bitDepthDataTypes)
			{
				if (bitDepthCase.second == this->hostBitDepth)
				{
					switch (bitDepthCase.first)
					{
					case TypeInt32Buffer:
						try
						{
							/* Uses additive synthesis to prepare the incoming buffer data */
							int* object = static_cast<int*>(y);
							int* amplitudes = static_cast<int*>(newAmplitudes);
							for (int i = 0; i < this->hostSampleRate; i++)
							{
								object[i] += amplitudes[i];
							}
							return true;
						}
						catch (exception ex)
						{
							printf(
								"The buffer Y array could not be additively adjusted. This is the exception that happened:\n%s\n", ex.what());
							return false;
						}
					default:
						throw exception("This datatype is not implemented.");
						return false;
					}
				}
			}
		}
		else
		{
			/* This indicates failure to allocate space. 
			   A failure like this is Likely due to a typedefinition being requested in an earlier sequence that doesn't exist */
			printf(
				"Error message: The buffer has no allocated space with which to place amplitudes.\n"
				"Error info: This indicates failure to allocate space.\n"
				"Error info: It is likely due to a typedefinition being requested that doesn't exist.\n"
			);
		}
	}

	template<typename dataType>
	bool IBuffer::tryToCallocY()
	{
		/*
			Initializes an array with zeros of any type assigned to a void pointer.
		*/
		try
		{
			this->y = (dataType*)calloc(this->hostSampleRate, sizeof(dataType));
			return true;
		}
		catch (exception ex)
		{
			throw exception(("The buffer could not calloc a Y array. This is the exception that happened:\n%s\n", ex.what()));
		}
	}

	template <typename dataType>
	bool IBuffer::resetArray(void* sourceArray, int value, int count)
	{
		/*
			Reallocates the data in a void array up to a specific point based on the type.
			This is used to ensure new or reset arrays do not have artifacts in them.
		*/
		try
		{
			dataType* object = static_cast<dataType*>(sourceArray);
			for (int i = 0; i < count; i++)
			{
				object[i] = value;
			}
			return true;
		}
		catch (exception ex)
		{
			printf("The buffer Y array could not be cleaned out. This is the exception that happened:\n%s\n", ex.what());
			return false;
		}
	}
}