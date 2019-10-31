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
 * ISDWavFile.h
 *
 *  Created on: Oct 31, 2019
 *      Author: JakeSoft
 */

#ifndef _ISDWAVFILE_H_
#define _ISDWAVFILE_H_

#include <SD.h>

struct tWavFileHeader
{
    char mChunkID[4];       //"RIFF" = 0x46464952
	uint32_t mChunkSize;    //28 [+ sizeof(wExtraFormatBytes) + wExtraFormatBytes] + sum(sizeof(chunk.id) + sizeof(chunk.size) + chunk.size)
	char mFormat[4];        //"WAVE" = 0x45564157
	char mSubchunk1ID[4];   //"fmt " = 0x20746D66
	uint32_t subchunk1Size; //16 [+ sizeof(wExtraFormatBytes) + wExtraFormatBytes]
	uint16_t audioFormat;
	uint16_t numChannels;
	uint32_t sampleRate;
	uint32_t byteRate;
	uint16_t blockAlign;
	uint16_t bitsPerSample;
};

struct tWavDataHeader
{
    char mID[4]; //"data" = 0x61746164
    uint32_t mSize;  //Chunk data bytes
};

//Interface class for wav files
class ISDWavFile
{
public:
	virtual ~ISDWavFile()
	{
		//Do nothing
	}

	/**
	 * Close the file.
	 */
	virtual void Close() = 0;

	/**
	 * Fetch the underlying file handle.
	 */
	virtual File& GetFileHandle() = 0;

	/**
	 * Fetch basic file header
	 */
	virtual const tWavFileHeader& GetHeader() = 0;

	/**
	 * Fetch header for the data block
	 */
	virtual const tWavDataHeader& GetDataHeader() = 0;

	/**
	 * Force the file's read pointer to the start of the data block
	 */
	virtual bool SeekStartOfData() = 0;

	/**
	 * Fetch how many bytes are available to be read before
	 * the read pointer reaches the end of the file.
	 * NOTE: This is not accurate until less then 32768 bytes
	 * are available in the file.
	 *
	 * Returns: Number of bytes left to be read
	 */
	virtual int Available() = 0;

	/**
	 * Fetch the sound data as 16-bit samples
	 * Args:
	 *   apBuffer - Pointer to buffer to fill with data
	 *   aNumSamples - How many samples to read
	 * Returns: Number of samples filled
	 */
	virtual int Fetch16BitSamples(int16_t* apBuffer, int aNumSamples) = 0;

	/**
	 * Set the output volume. This can be used to adjust the
	 * relative volume of each file when multiple files are played
	 * at the same time.
	 *
	 * Args:
	 *   aVolume - Any value between 1.0 (max) and 0.0 (mute)
	 */
	virtual void SetVolume(float aVolume) = 0;

	/**
	 * Enable/Disable looping. When looping is enabled and the end of file is
	 * reached, the read pointer will automatically reset to the start of
	 * the data block. This has the effect of simulating a wav file that
	 * never runs out of data.
	 *
	 * Args:
	 *   aLoopingEnable - TRUE = Do looping, FALSE = Play once, no looping
	 */
	virtual void SetLooping(bool aLoopingEnable) = 0;

	/**
	 * Sets the paused flag. See IsPaused().
	 */
	virtual void Pause() = 0;

	/**
	 * Check if this file is paused.
	 *
	 * Return: TRUE if paused, FALSE otherwise
	 */
	virtual bool IsPaused() = 0;

	/**
	 * Clears the paused flag. See IsPaused().
	 */
	virtual void UnPause() = 0;

	/**
	 * Check if file has run out of data.
	 * NOTE: This will always be false if looping is enabled.
	 * Returns: TRUE if file has run out of data, FALSE otherwise.
	 */
	virtual bool IsEnded() = 0;

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
	virtual void SetDePop(bool aStart, bool aEnd) = 0;

	/**
	 * Skips samples. All read pointers are advanced.
	 * Args:
	 *  aNumSamples - Number of 16-bit samples to skip.
	 */
	virtual void Skip16BitSamples(int aNumSamples) = 0;

};

#endif /* _ISDWAVFILE_H_ */
