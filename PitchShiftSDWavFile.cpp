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
	mPlaybackRate = 0;
	mCurSample = 0;
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
	//Fetch the next sample as usual, no rate change
	if(mPlaybackRate == 0 || mSampleIndex == 0)
	{
		SDWavFile::Fetch16BitSamples(&mCurSample, 1);
	}
	//Slow down playback by repeating samples
	else if(mPlaybackRate < 0)
	{
		if(mSampleIndex % mShiftVars.mNumSamplesToRepeat == 0)
		{
			//Do nothing, just use the already buffered sample
		}
		else
		{
			SDWavFile::Fetch16BitSamples(&mCurSample, 1);
			mShiftVars.mSamplesRepeatedCounter = 0;
		}
	}
	//Speed up playback by skipping samples
	else if(mPlaybackRate > 0)
	{
		Skip16BitSamples(mShiftVars.mNumSamplesToSkip);
		SDWavFile::Fetch16BitSamples(&mCurSample, 1);
	}

	mSampleIndex++;
}

void PitchShiftSDWavFile::SetRate(int aRate)
{
	mPlaybackRate = aRate;

	mShiftVars.mNumSamplesToRepeat = 0;
	mShiftVars.mNumSamplesToSkip = 0;

	if(mPlaybackRate < 0)
	{
		mShiftVars.mNumSamplesToRepeat = 8 - abs(mPlaybackRate);
		mShiftVars.mNumSamplesToSkip = 0;
	}
	else if(mPlaybackRate > 0)
	{
		mShiftVars.mNumSamplesToRepeat = 0;
		mShiftVars.mNumSamplesToSkip = aRate;
	}
}
