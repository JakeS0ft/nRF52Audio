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
 * PitchShiftedSDWavFile.cpp
 *
 *  Created on: Aug 31, 2019
 *      Author: JakeSoft
 */

#include "PitchShiftSDWavFile.h"

PitchShiftSDWavFile::PitchShiftSDWavFile(const char* aFilePath)
: SDWavFile(aFilePath) //Call superclass constructor
{
	mCurSample = 0;

	mSkipFactor = 0.0;
	mRepeatFactor = 0.0;
	mRepeatAccumulator = 0.0;
	mSkipAccumulator = 0.0;
}

PitchShiftSDWavFile::~PitchShiftSDWavFile()
{
	if(nullptr != mpFileReader)
	{
		delete mpFileReader;
		mFileHandle.close();
	}
}

int PitchShiftSDWavFile::Fetch16BitSamples(int16_t* apBuffer, int aNumSamples)
{
	int lSampleIndex = 0;

	for(lSampleIndex = 0;
		lSampleIndex < aNumSamples && !IsEnded();
		lSampleIndex++)
	{
		CalculateCurSample();
		apBuffer[lSampleIndex] = mCurSample;
	}

	return lSampleIndex;
}

void PitchShiftSDWavFile::CalculateCurSample()
{
	//Always fetch the first sample
	if(0 == mSampleIndex)
	{
		SDWavFile::Fetch16BitSamples(&mCurSample, 1);
	}
	//We want to skip samples to speed up playback
	else if(mSkipFactor > 0.0)
	{
		if(mSkipAccumulator >= 1.0)
		{
			while(mSkipAccumulator >= 1.0)
			{
				SDWavFile::Skip16BitSamples(1);
				mSkipAccumulator -= 1.0;
			}
		}
		else
		{
			mSkipAccumulator += mSkipFactor;
		}

		SDWavFile::Fetch16BitSamples(&mCurSample, 1);
	}
	//We want to repeat samples to slow down playback
	else if(mRepeatFactor > 0.0)
	{
		if(mRepeatAccumulator >= 1.0)
		{
			//Repeat last sample (don't update mCurSample) and adjust the accumulator
			mRepeatAccumulator =- 1.0;
		}
		else
		{
			//Fetch the next sample and adjust the accumulator
			mRepeatAccumulator += mRepeatFactor;
			SDWavFile::Fetch16BitSamples(&mCurSample, 1);
		}
	}
	//Play at normal speed
	else
	{
		SDWavFile::Fetch16BitSamples(&mCurSample, 1);
	}

	mSampleIndex++;
}

void PitchShiftSDWavFile::SetRate(float aRate)
{
	//Slow it down
	if(aRate < 0.0)
	{
		mRepeatFactor = -1.0 * aRate; //Flips negative value to positive

		//Turn off the stuff that would speed up playback
		mSkipFactor = 0.0;
		mSkipAccumulator = 0.0;
	}
	//Speed it up
	else if(aRate > 0.0)
	{
		mSkipFactor = aRate;

		//Turn off the stuff that would slow down playback
		mRepeatFactor = 0.0;
		mRepeatAccumulator = 0.0;
	}
	//Play at normal speed
	else
	{
		mRepeatFactor = 0.0;
		mRepeatAccumulator = 0.0;
		mSkipFactor = 0.0;
		mSkipAccumulator = 0.0;
	}
}
