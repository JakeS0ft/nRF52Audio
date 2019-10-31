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
 * ChainedSDWavFile.h
 *
 *  Created on: Oct 31, 2019
 *      Author: JakeSoft
 */

#ifndef _CHAINEDSDWAVFILE_H_
#define _CHAINEDSDWAVFILE_H_

#include "ISDWavFile.h"
#include "SDWavFile.h"

#define NUM_CHAINED_FILES 2

/**
 * This class allows for seamless (gapless) transition between two wav files.
 * The first file is played until it ends, then the second file starts
 * right after the first is done. The second file can be optionally looped by
 * calling the SetLooping() method.
 * NOTE: It is not recommend to mix files with different sample rates in a chain.
 */
class ChainedSDWavFile : public ISDWavFile
{
public:

	/**
	 * Constructor.
	 * Args:
	 *  aFilePath - First file to play
	 *  aNextFilePath - File to play when the first file is done
	 */
	ChainedSDWavFile(const char* aFilePath, const char* aNextFilePath);

	/**
	 * Descturctor.
	 */
	~ChainedSDWavFile();

	/**
	 * Fetch the underlying file handle.
	 * NOTE: Fetches the handle of the current file being played.
	 * If first file has finished playback, then this will return
	 * the file handle for the second file.
	 */
	File& GetFileHandle();

	/**
	 * Fetch basic file header.
	 * NOTE: Fetches the file header of the curent file being played.
	 */
	const tWavFileHeader& GetHeader();

	/**
	 * Fetch header for the data block
	 */
	const tWavDataHeader& GetDataHeader();

	/**
	 * Close the files.
	 */
	virtual void Close();

	/**
	 * Force the file's read pointer to the start of the data block
	 */
	virtual bool SeekStartOfData();

	/**
	 * Fetch how many bytes are available to be read before
	 * the read pointer reaches the end of the file.
	 * NOTE: This is not accurate until less then 32768 bytes
	 * are available in the file.
	 *
	 * Returns: Number of bytes left to be read
	 */
	virtual int Available();

	/**
	 * Fetch the sound data as 16-bit samples
	 * Args:
	 *   apBuffer - Pointer to buffer to fill with data
	 *   aNumSamples - How many samples to read
	 * Returns: Number of samples filled
	 */
	virtual int Fetch16BitSamples(int16_t* apBuffer, int aNumSamples);

	/**
	 * Set the output volume. This can be used to adjust the
	 * relative volume of each file when multiple files are played
	 * at the same time.
	 *
	 * Args:
	 *   aVolume - Any value between 1.0 (max) and 0.0 (mute)
	 */
	virtual void SetVolume(float aVolume);

	/**
	 * Enable/Disable looping. When looping is enabled and the end of file is
	 * reached, the read pointer will automatically reset to the start of
	 * the data block. This has the effect of simulating a wav file that
	 * never runs out of data.
	 *
	 * Args:
	 *   aLoopingEnable - TRUE = Do looping, FALSE = Play once, no looping
	 */
	virtual void SetLooping(bool aLoopingEnable);

	/**
	 * Sets the paused flag. See IsPaused().
	 */
	virtual void Pause();

	/**
	 * Check if this file is paused.
	 *
	 * Return: TRUE if paused, FALSE otherwise
	 */
	virtual bool IsPaused();

	/**
	 * Clears the paused flag. See IsPaused().
	 */
	virtual void UnPause();

	/**
	 * Check if file has run out of data.
	 * NOTE: This will always be false if looping is enabled.
	 * Returns: TRUE if file has run out of data, FALSE otherwise.
	 */
	virtual bool IsEnded();

	/**
	 * Enable/Disable the De-pop algorithm.
	 * The De-pop alogrithm attempts to eliminate pops that can occur when
	 * a file first starts or when playback loops from the end to the start.
	 * When looping is enabled, it is recommended that both start and end
	 * de-pop be enabled.
	 * Args:
	 *   aStart - TRUE= Enable for start of file, FALSE = disabled
	 *   aEnd - TRUE = Enable for end of file, FALSE = disabled
	 */
	virtual void SetDePop(bool aStart, bool aEnd);

	/**
	 * Skips samples. All read pointers are advanced.
	 * Args:
	 *  aNumSamples - Number of 16-bit samples to skip.
	 */
	virtual void Skip16BitSamples(int aNumSamples);

protected:

	SDWavFile* mpFiles[NUM_CHAINED_FILES];
	int mFileIndex;
};




#endif /* _CHAINEDSDWAVFILE_H_ */
