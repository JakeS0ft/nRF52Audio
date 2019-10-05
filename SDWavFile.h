/*
 * SDWavFile.h
 *
 *  Created on: May 30, 2019
 *      Author: JakeSoft
 */

#ifndef SDWAVFILE_H_
#define SDWAVFILE_H_

#include <Arduino.h>
#include <SD.h>
#include "BufferedFileReader.h"

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

/**
 * This class represents a single .wav file on an SD card. It is
 * responsible for opening the file, reading the data, and making
 * the samples available.
 */
class SDWavFile
{
public:
	/**
	 * Constructor.
	 * Args:
	 *  aFilePath - Name of file to read
	 */
	SDWavFile(const char* aFilePath);

	/**
	 * Destructor.
	 */
	virtual ~SDWavFile();

	/**
	 * Fetch the underlying file handle.
	 */
	File& GetFileHandle();

	/**
	 * Fetch basic file header
	 */
	const tWavFileHeader& GetHeader();

	/**
	 * Fetch header for the data block
	 */
	const tWavDataHeader& GetDataHeader();

	/**
	 * Close the file.
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
	void SetVolume(float aVolume);

	/**
	 * Enable/Disable looping. When looping is enabled and the end of file is
	 * reached, the read pointer will automatically reset to the start of
	 * the data block. This has the effect of simulating a wav file that
	 * never runs out of data.
	 *
	 * Args:
	 *   aLoopingEnable - TRUE = Do looping, FALSE = Play once, no looping
	 */
	void SetLooping(bool aLoopingEnable);

	/**
	 * Sets the paused flag. See IsPaused().
	 */
	void Pause();

	/**
	 * Check if this file is paused.
	 *
	 * Return: TRUE if paused, FALSE otherwise
	 */
	bool IsPaused();

	/**
	 * Clears the paused flag. See IsPaused().
	 */
	void UnPause();

	/**
	 * Fetch total number of files open globally.
	 */
	inline static int GetNumFilesOpen()
	{
		return sFilesOpen;
	}

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

	/**
	 * Default consturctor. Made protected so subclasses
	 * don't have to deal with the normal constructer args.
	 */
	SDWavFile();

	/**
	 * Read and store the wav file header.
	 */
	void ReadHeader();

	/**
	 * Read and store the data block header.
	 */
	void ReadDataHeader();

	/**
	 * Byte swap the 16-bit words in an I2S sample
	 */
	void ByteSwapI2SSample(int32_t* aSample);

	//File path
	const char* mpFilePath;

	//Wav file header data
	tWavFileHeader mHeader;
	//Data block header
	tWavDataHeader mDataHeader;

	//File handle
	File mFileHandle;

	//Bytes per sample (should always be 2)
	int mBytesPerSample;

	//Volume (0.0 to 1.0)
	float mVolume;

	//Looping flag
	bool mIsLooping;

	//Paused flag
	bool mIsPaused;

	//Stopped flag
	bool mIsStopped;

	//Keep track of how many Wav files are opened globally
	static int sFilesOpen;

	//Manage buffering the file data
	BufferedFileReader* mpFileReader;

	//Last fetched sample
	int16_t mLastSample;

	//Keep track of how many samples were read
	unsigned long mSamplesRead;

	//Apply de-pop to start of file
	bool mDepopStart;

	//Apply de-pop to end of file
	bool mDepopEnd;

};

#endif /* SDWAVFILE_H_ */
