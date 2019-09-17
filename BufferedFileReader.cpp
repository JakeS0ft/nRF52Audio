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
 * BufferedFileReader.cpp
 *
 *  Created on: Jul 8, 2019
 *      Author: JakeSoft
 */

#include "BufferedFileReader.h"
#include "Arduino.h"

BufferedFileReader::BufferedFileReader(File* apFileHandle)
{
	mpFileHandle = apFileHandle;
	mDataBufferPos = 0;
	mDataBufferAvailableBytes = 0;
	mDataBlock = 0;
}

BufferedFileReader::~BufferedFileReader()
{

}

int BufferedFileReader::BufferAvailable()
{
	return mDataBufferAvailableBytes;
}

int BufferedFileReader::FetchBufferedBytes(int8_t* apOutBuffer, int aSize)
{
	int lActualByteCount = 0;

	memset(apOutBuffer, 0, aSize);

//	if(aSize <= mDataBufferAvailableBytes)
//	{
//		memcpy(apOutBuffer, mDataBytes, aSize);
//		mDataBufferPos += aSize;
//		lActualByteCount = aSize;
//
//		if(mDataBufferPos >= DATA_BLOCK_SIZE)
//		{
//			if(mpFileHandle->available() > 0)
//			{
//				ReadNextDataBlock();
//			}
//			else
//			{
//				mDataBufferAvailableBytes = 0;
//			}
//		}
//	}

	for(int lIdx = 0;
		lIdx < aSize && mDataBufferAvailableBytes > 0 && mDataBufferPos < DATA_BLOCK_SIZE;
		lIdx++)
	{
		//Serial.println("Fetching byte.");
		apOutBuffer[lIdx] = mDataBytes[mDataBufferPos];
		mDataBufferPos++;
		lActualByteCount++;
		mDataBufferAvailableBytes--;

		if(mDataBufferPos >= DATA_BLOCK_SIZE)
		{
			if(mpFileHandle->available() > 0)
			{
				ReadNextDataBlock();
			}
			else
			{
				mDataBufferAvailableBytes = 0;
			}
		}
	}

	return lActualByteCount;
}

void BufferedFileReader::ReadNextDataBlock()
{
//	Serial.println("Reading data block");
	//Clear out any existing data
	memset(mDataBytes, 0, DATA_BLOCK_SIZE);

	if(mpFileHandle->available())
	{
		mDataBlock++;
	}
	mDataBufferPos = 0;
	mDataBufferAvailableBytes = 0;
	if(mpFileHandle->available() >= DATA_BLOCK_SIZE)
	{
		mpFileHandle->read(mDataBytes, DATA_BLOCK_SIZE);
		mDataBufferAvailableBytes = DATA_BLOCK_SIZE;
	}
	else
	{
		int lNumBytes = mpFileHandle->available();
		mpFileHandle->read(mDataBytes, lNumBytes);
		mDataBufferAvailableBytes = lNumBytes;
	}

}
