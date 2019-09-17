/*
 * SDWavFile.cpp
 *
 *  Created on: May 30, 2019
 *      Author: JakeSoft
 */


#include "SDWavFile.h"
#include <SD.h>
#include <Arduino.h>

#define DATA_START_OFFSET 44 //Byte offset where the data always starts

int SDWavFile::sFilesOpen = 0;

SDWavFile::SDWavFile(const char* apFilePath)
{
	//Store the file path
	mpFilePath = apFilePath;
	mVolume = 1.0;
	mIsLooping = false;
	mIsPaused = false;

	mFileHandle = SD.open(apFilePath, FILE_READ);
	sFilesOpen++;

	mpFileReader = new BufferedFileReader(&mFileHandle);
	mpFileReader->Reset();

	ReadHeader();
	ReadDataHeader();
	//SeekStartOfData();
}

SDWavFile::SDWavFile()
{
	//Store the file path
	mpFilePath = "";
	mVolume = 1.0;
	mIsLooping = false;
	mIsPaused = false;
	mpFileReader = nullptr;
	mIsStopped = false;
	mBytesPerSample = 2;
}

SDWavFile::~SDWavFile()
{
	if(nullptr != mpFileReader)
	{
		delete mpFileReader;
		mFileHandle.close();
	}
}

File& SDWavFile::GetFileHandle()
{
	return mFileHandle;
}

const tWavFileHeader& SDWavFile::GetHeader()
{
	return mHeader;
}

const tWavDataHeader& SDWavFile::GetDataHeader()
{
	return mDataHeader;
}

void SDWavFile::Close()
{
	mFileHandle.close();
	sFilesOpen--;
}

bool SDWavFile::SeekStartOfData()
{
	if(mFileHandle.size() >= DATA_START_OFFSET+1)
	{
		//lSuccess = mFileHandle.seek(DATA_START_OFFSET);
		mpFileReader->Reset();
		mpFileReader->BufferSeek(DATA_START_OFFSET);
	}

	return true;
}

int SDWavFile::Available()
{
	if(mpFileReader->BufferAvailable() > mFileHandle.available())
	{
		return mFileHandle.available();
	}
	else
	{
		return mpFileReader->BufferAvailable();
	}
}

int SDWavFile::Fetch16BitSamples(int16_t* apBuffer, int aNumSamples)
{
	//Read until requested size is met or we run out of data
	int lSampleIndex = 0;
	for(lSampleIndex = 0;
		lSampleIndex < aNumSamples &&
		(mFileHandle.available() >= sizeof(int16_t) || mpFileReader->BufferAvailable() >= sizeof(int16_t));
		lSampleIndex++)
	{
		//Clear current sample in the output buffer
		apBuffer[lSampleIndex] = 0;

		//Read 2 bytes (16 bits) sample from the file
		mpFileReader->FetchBufferedBytes((int8_t*)&apBuffer[lSampleIndex], 2);

		//Apply volume by reducing the amplitude of each sample
		if(mVolume < 1.0 && mVolume >= 0.0)
		{
			apBuffer[lSampleIndex] = apBuffer[lSampleIndex] * mVolume;
		}

		//If we ran out of data, check if we should loop back to the start
		if(mFileHandle.available() < sizeof(int16_t)
				&& mpFileReader->BufferAvailable() < sizeof(int16_t)
				&& true == mIsLooping)
		{
				SeekStartOfData();
		}

	}

	return lSampleIndex;
}

void SDWavFile::SetVolume(float aVolume)
{
	if(aVolume <= 0.0)
	{
		mVolume = 0.0;
	}
	else if(aVolume >= 1.0)
	{
		mVolume = 1.0;
	}
	else
	{
		mVolume = aVolume;
	}
}

void SDWavFile::SetLooping(bool aLoopingEnable)
{
	mIsLooping = aLoopingEnable;
}

void SDWavFile::Pause()
{
	mIsPaused = true;
}

bool SDWavFile::IsPaused()
{
	return mIsPaused;
}

void SDWavFile::UnPause()
{
	mIsPaused = false;
}

bool SDWavFile::IsEnded()
{
	bool lbEnded = false;

	if(mpFileReader->IsEnded() && !mIsLooping)
	{
		lbEnded = true;
	}

	return lbEnded;
}

void SDWavFile::ReadHeader()
{
	if(mpFileReader->BufferAvailable() >= sizeof(tWavFileHeader) )
	{
		int8_t* lpReadByte = (int8_t*)&mHeader;
		mpFileReader->FetchBufferedBytes(lpReadByte, sizeof(tWavFileHeader));
	}

	mBytesPerSample = (mHeader.bitsPerSample / 8);
}

void SDWavFile::ReadDataHeader()
{
	if(mpFileReader->BufferAvailable() >= sizeof(tWavDataHeader))
	{
		int8_t* lpReadByte = (int8_t*)&mDataHeader;
		mpFileReader->FetchBufferedBytes(lpReadByte, sizeof(tWavDataHeader));
	}
}

void SDWavFile::ByteSwapI2SSample(int32_t* apSample)
{
	//4-byte temporary buffer to hold the raw 4 byte (32 bits) I2S sample
	uint8_t lpTempBytes[4];
	//1-byte pointer so we can manipulate the 4 byte input sample one byte at a time
	uint8_t* lpSampleBytePtr = (uint8_t*)apSample;

	//Store raw sample bytes in temporary byte buffer
	memcpy(&lpTempBytes[0], &apSample[0], sizeof(uint32_t));

	//Swap bytes 0 and 1 (Channel 1)
	memcpy(&lpSampleBytePtr[1], &lpTempBytes[0], 1);
	memcpy(&lpSampleBytePtr[0], &lpTempBytes[1], 1);
	//Swap bytes 2 and 3 (Channel 2)
	memcpy(&lpSampleBytePtr[2], &lpTempBytes[3], 1);
	memcpy(&lpSampleBytePtr[3], &lpTempBytes[2], 1);
}
