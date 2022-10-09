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

	IBuffer::IBuffer(int definitions[], int count)
	{
		/* Since various libraries use different define values, this allows each buffer to inject their own as a map. */
		for (int i = 0; i < count; i++)
		{
			this->bitDepthDataTypes.insert({ i + 1, definitions[i] });
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
							this->resetArray<int>(this->y, 0, this->hostSampleRate) : this->tryToCallocY<int>();
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
		}
	}
}