/******************************************************************************
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 ******************************************************************************/

/*
 * BufferedFileReader.h
 *
 *  Created on: Jul 8, 2019
 *      Author: JakeSoft
 */

#ifndef BUFFEREDFILEREADER_H_
#define BUFFEREDFILEREADER_H_

#include <SD.h>

#define DATA_BLOCK_SIZE 1024

/**
 * This class is responsible for reading data from a file on an SD card
 * and buffering the bytes for future processing. This class is necessary
 * for concurrent reads of SD file data because SD FAT will throw out
 * entire data blocks (512 or 1024 bytes depending on the card) if the
 * entire block is not read at once.
 *
 * So, for example, if you read 2 bytes from FileA, then 2 byes from FileB,
 * then try to read the next two bytes from FileA, the FileA read will give zeros
 * because it tossed out the entire data block when FileB was read.
 */
class BufferedFileReader
{
public:
	/**
	 * Constructor
	 * Args:
	 *  apFileHandle - Pointer to file to source data from
	 */
	BufferedFileReader(File* apFileHandle);

	/**
	 * Destructor
	 */
	~BufferedFileReader();

	/**
	 * See how much data is available in the buffer. This winds down as
	 * FetchBufferedBytes() is called, then resets to a larger number
	 * when the next data block is read.
	 *
	 * Returns: Number of bytes available to be fetched from the buffer
	 */
	int BufferAvailable();

	/**
	 * Fetch bytes from the buffer.
	 * Args:
	 *  apOutBuffer - Buffer to copy data into
	 *  aSize - How many bytes to copy
	 *
	 * Returns: Number of valid bytes put in the output buffer. Usually this will be the
	 *   same as the number of bytes requested, however if the file runs out
	 *   of data then this will be something less.
	 *
	 */
	int FetchBufferedBytes(int8_t* apOutBuffer, int aSize);

	/**
	 * Resets all position tracking and starts reading from the start of the
	 * file again. This is as if the file was closed and reopened again,
	 * although the file handle is never actually closed.
	 */
	inline void Reset()
	{
		mDataBufferPos = 0;
		mDataBlock = 0;
		mpFileHandle->seek(0);

		ReadNextDataBlock();
	}

	/**
	 * Fetch a counter that keeps track of how many data blocks have been read.
	 */
	inline int GetDataBlockPos()
	{
		return mDataBlock;
	}

	/**
	 * Fetch current read index of the data buffer. This will be a number between
	 * 0 and DATA_BLOCK_SIZE-1.
	 */
	inline int GetBufferPos()
	{
		return mDataBufferPos;
	}

	/**
	 * Manually set the read index of the data buffer. This can be used to skip
	 * around in the buffered data.
	 * Args:
	 *   aPos - New position index. Valid values are between 0 and DATA_BLOCK_SIZE-1.
	 */
	inline int BufferSeek(int aPos)
	{
		if(aPos < DATA_BLOCK_SIZE)
		{
			if(aPos > mDataBufferPos)
			{
				mDataBufferAvailableBytes -= (aPos - mDataBufferPos);
			}
			else if (aPos < mDataBufferPos)
			{
				mDataBufferAvailableBytes += (mDataBufferPos - aPos);
			}

			mDataBufferPos = aPos;
		}

		return mDataBufferAvailableBytes;
	}

	/**
	 * Indicates if we have run out of data in both the file and the buffer.
	 *
	 * Returns: TRUE if out of data, FALSE otherwise.
	 */
	inline bool IsEnded()
	{
		bool lbIsEnded = false;

		if(0 == mDataBufferAvailableBytes && !mpFileHandle->available())
		{
			lbIsEnded = true;
		}

		return lbIsEnded;
	}

	/**
	 * Fetch a raw pointer to the data buffer. This will be a pointer to an
	 * array of raw bytes of size DATA_BLOCK_SIZE. This method should be used
	 * sparingly to directly manipulate the buffered data.
	 */
	inline int8_t* GetDataBuffer()
	{
		return mDataBytes;
	}

	/**
	 * Fetch the size of the data buffer.
	 * Returns: Size of the data buffer.
	 */
	inline int GetDataBufferSize()
	{
		return DATA_BLOCK_SIZE;
	}

protected:

	/**
	 * Reads the next DATA_BLOCK_SIZE bytes into the data buffer overwriting
	 * the old data. If the file does not have enough data to fill the
	 * buffer then the extra buffer space will be filled with zeros.
	 */
	void ReadNextDataBlock();

	//File handle to read data from
	File* mpFileHandle;

	//Buffered data bytes
	int8_t mDataBytes[DATA_BLOCK_SIZE];
	//Data buffer read position
	int mDataBufferPos;
	//Data block counter
	int mDataBlock;
	//How many valid bytes of data are available in the data buffer
	int mDataBufferAvailableBytes;
};



#endif /* BUFFEREDFILEREADER_H_ */
