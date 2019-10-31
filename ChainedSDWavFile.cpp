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
 * ChainedSDWavFile.cpp
 *
 *  Created on: Oct 31, 2019
 *      Author: JakeSoft
 */

#include "ChainedSDWavFile.h"

ChainedSDWavFile::ChainedSDWavFile(const char* aFilePath, const char* aNextFilePath)
{
	mFileIndex = 0;

	mpFiles[0] = new SDWavFile(aFilePath);
	mpFiles[1] = new SDWavFile(aNextFilePath);
}

ChainedSDWavFile::~ChainedSDWavFile()
{
	for(int lIdx = 0; lIdx < NUM_CHAINED_FILES; lIdx++)
	{
		delete mpFiles[lIdx];
	}
}

File& ChainedSDWavFile::GetFileHandle()
{
	return mpFiles[mFileIndex]->GetFileHandle();
}

const tWavFileHeader& ChainedSDWavFile::GetHeader()
{
	return mpFiles[mFileIndex]->GetHeader();
}

const tWavDataHeader& ChainedSDWavFile::GetDataHeader()
{
	return mpFiles[mFileIndex]->GetDataHeader();
}

void ChainedSDWavFile::Close()
{
	for(int lIdx = 0; lIdx < NUM_CHAINED_FILES; lIdx++)
	{
		mpFiles[lIdx]->Close();
	}
}

bool ChainedSDWavFile::SeekStartOfData()
{
	bool lSuccess = true;
	for(int lIdx = 0; lIdx < NUM_CHAINED_FILES; lIdx++)
	{
		lSuccess &= mpFiles[lIdx]->SeekStartOfData();
	}

	return lSuccess;
}

int ChainedSDWavFile::Available()
{
	int lAvail = 0;

	//Return the greatest available bytes from either file
	for(int lIdx = 0; lIdx < NUM_CHAINED_FILES; lIdx++)
	{
		if(mpFiles[lIdx]->Available() > lAvail)
		{
			lAvail = mpFiles[lIdx]->Available();
		}
	}

	return lAvail;
}

int ChainedSDWavFile::Fetch16BitSamples(int16_t* apBuffer, int aNumSamples)
{
	int lSamplesFetched = 0;

	static int lWrap = 0;

	if(false == mpFiles[0]->IsEnded())
	{
		lSamplesFetched = mpFiles[0]->Fetch16BitSamples(apBuffer, aNumSamples);
	}

	if(lSamplesFetched < aNumSamples)
	{
		int lNumSamplesToFetch =  aNumSamples-lSamplesFetched;
		int lBufPos = 0;
		if(lSamplesFetched > 0)
		{
			lBufPos = lSamplesFetched-1;
		}

		lSamplesFetched += mpFiles[1]->Fetch16BitSamples(&(apBuffer[lBufPos]), lNumSamplesToFetch);
	}

	return lSamplesFetched;
}

void ChainedSDWavFile::SetVolume(float aVolume)
{
	for(int lIdx = 0; lIdx < NUM_CHAINED_FILES; lIdx++)
	{
		mpFiles[lIdx]->SetVolume(aVolume);
	}
}

void ChainedSDWavFile::SetLooping(bool aLoopingEnable)
{
	mpFiles[0]->SetLooping(0);
	mpFiles[1]->SetLooping(aLoopingEnable);
}

void ChainedSDWavFile::Pause()
{
	for(int lIdx = 0; lIdx < NUM_CHAINED_FILES; lIdx++)
	{
		mpFiles[lIdx]->Pause();
	}
}

bool ChainedSDWavFile::IsPaused()
{
	return mpFiles[0]->IsPaused();
}

void ChainedSDWavFile::UnPause()
{
	for(int lIdx = 0; lIdx < NUM_CHAINED_FILES; lIdx++)
	{
		mpFiles[lIdx]->UnPause();
	}
}

bool ChainedSDWavFile::IsEnded()
{
	bool lEnded = true;
	for(int lIdx = 0; lIdx < NUM_CHAINED_FILES; lIdx++)
	{
		if(false == mpFiles[lIdx]->IsEnded())
		{
			lEnded = false;
		}
	}

	return lEnded;
}

void ChainedSDWavFile::SetDePop(bool aStart, bool aEnd)
{
	for(int lIdx = 0; lIdx < NUM_CHAINED_FILES; lIdx++)
	{
		mpFiles[lIdx]->SetDePop(aStart, aEnd);
	}
}

void ChainedSDWavFile::Skip16BitSamples(int aNumSamples)
{
	mpFiles[mFileIndex]->Skip16BitSamples(aNumSamples);
}

