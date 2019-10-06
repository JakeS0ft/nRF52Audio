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
	 *  aRate - A value from that defines how much the pitch will change.
	 */
	virtual void SetRate(float aRate);
protected:

	/**
	 * Calculates the next sample based on current pitch shift settings.
	 * Stores the result in class member data.
	 */
	void CalculateCurSample();

	//Current sample
	int16_t mCurSample;

	//Sample index, increments each time a sample is calculated
	unsigned long mSampleIndex;

	//How much to increment the Skip Accumulator each time a sample is read
	float mSkipFactor;
	//Increments each time a sample is read, decrements when a sample is skipped
	float mSkipAccumulator;

	//How much to increment the Repeat Accumulator each time a sample is read
	float mRepeatFactor;
	//Increments each time a sample is read, decrements when sample is repeated
	float mRepeatAccumulator;
};

#endif /* PITCHSHIFTSDWAVFILE_H_ */
