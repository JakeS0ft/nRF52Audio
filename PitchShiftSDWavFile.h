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
 * PitchShiftSDWaveFile.h
 *
 *  Created on: Aug 31, 2019
 *      Author: JakeSoft
 */

#ifndef PITCHSHIFTSDWAVFILE_H_
#define PITCHSHIFTSDWAVFILE_H_

#include <SDWavFile.h>

/**
 * Special type of SDWavFile with and added function SetRate() that allows
 * the pitch to be shifted by selectively skipping or repeating samples
 * during playback. Setting a rate of 0 will have not effect and the
 * file will play normally.
 */
class PitchShiftSDWavFile : public SDWavFile
{
public:

	/**
	 * Constructor.
	 * Args:
	 *  aFilePath - Name of file to read
	 */
	PitchShiftSDWavFile(const char* aFilePath);

	/**
	 * Destructor.
	 */
	virtual ~PitchShiftSDWavFile();

	/**
	 * Fetch the sound data as 16-bit samples
	 * Args:
	 *   apBuffer - Pointer to buffer to fill with data
	 *   aNumSamples - How many samples to read
	 * Returns: Number of samples filled
	 */
	virtual int Fetch16BitSamples(int16_t* apBuffer, int aNumSamples);

	/**
	 * Sets the pitch rate. Negative values will decrease the pitch, positive
	 * values will increase the pitch.
	 * Args:
	 *  aRate - A value from -8 to 8 that defines how much the pitch will change
	 */
	virtual void SetRate(int aRate);
protected:

	//Data structure to hold pitch shift variables
	struct tPitchShiftVariables
	{
		//Number of samples to skip
		unsigned int mNumSamplesToSkip = 0;
		//Keep track of how many samples have been skipped
		int mSamplesSkippedCounter = 0;

		//Number of samples ot repeat
		unsigned int mNumSamplesToRepeat = 0;
		//Keep track of how any samples have been repeated
		int mSamplesRepeatedCounter = 0;

		//Multiplier for how many samples should be skipped or repeated
		unsigned int mMultiplicity = 0;;
	};


	/**
	 * Calculates the next sample based on current pitch shift settings.
	 * Stores the result in class member data.
	 */
	void CalculateCurSample();

	//Number of samples to skip or repeat from each data block
	int mPlaybackRate;

	//Current sample
	int16_t mCurSample;

	//Sample index, increments each time a sample is fetched by Fetch16BitSamples()
	unsigned long mSampleIndex;

	//Keep track of pitch-shift variables
	tPitchShiftVariables mShiftVars;
};

#endif /* PITCHSHIFTSDWAVFILE_H_ */
