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
 * I2SWavPlayer.cpp
 *
 *  Created on: Jun 30, 2019
 *      Author: JakeSoft
 */

#include "Arduino.h"
#include "I2SWavPlayer.h"

I2SWavPlayer::I2SWavPlayer(int32_t aPinMCK,
	     	 	 	 	  int32_t aPinBCLK,
						  int32_t aPinLRCK,
						  int32_t aPinDIN,
						  int32_t aPinSD)
{
	memset(maBufferA, 0, I2S_BUF_SIZE);
	memset(maBufferB, 0, I2S_BUF_SIZE);
	mBufferASelected = true;

	mPinMCK = aPinMCK;
	mPinBCLK = aPinBCLK;
	mPinLRCK = aPinLRCK;
	mPinDIN = aPinDIN;
	mPinSD = aPinSD;

	for(int lIdx = 0; lIdx < MAX_WAV_FILES; lIdx++)
	{
		mapWavFile[lIdx] = nullptr;
	}

	mSamplesMixed = 0;
	mSampleRate = ee2205;
	mVolume = 1.0;

}

I2SWavPlayer::~I2SWavPlayer()
{
	StopPlayback();

	for(int lIdx = 0; lIdx < MAX_WAV_FILES; lIdx++)
	{
		if(nullptr != mapWavFile[lIdx])
		{
			mapWavFile[lIdx]->Close();
		}
	}
}

bool I2SWavPlayer::Init()
{
	Configure_I2S();
	return true;
}

void I2SWavPlayer::SetWavFile(SDWavFile* apWavFile, int aFileIndex)
{
	if(aFileIndex < MAX_WAV_FILES && aFileIndex >= 0)
	{
		mapWavFile[aFileIndex] = apWavFile;
		if(nullptr != apWavFile)
		{
			//Force down-sampling of 44.1KHz to 22.05 KHz
			if(apWavFile->GetHeader().sampleRate == 44100 && mSampleRate == ee2205)
			{
				//Serial.println("Downsampling enabled.");
				maDownsampleParams[aFileIndex].mIsDownsample = true;
			}
			else //Don't do down-sampling
			{
				//Serial.println("Downsampling disabled.");
				maDownsampleParams[aFileIndex].mIsDownsample = false;
			}

			//Serial.print("SampleRate: ");
			//Serial.println(apWavFile->GetHeader().sampleRate);

			apWavFile->SeekStartOfData();


		}
	}
}

void  I2SWavPlayer::ClearAllWavFiles()
{
	for(int lIdx = 0; lIdx < MAX_WAV_FILES; lIdx++)
	{
		mapWavFile[lIdx] = nullptr;
	}

	//Flush the I2S buffers so only silence will play
	memset(maBufferA, 0, sizeof(int32_t)*I2S_BUF_SIZE);
	memset(maBufferB, 0, sizeof(int32_t)*I2S_BUF_SIZE);
}

void I2SWavPlayer::StartPlayback()
{

	for(int lIdx = 0; lIdx < I2S_BUF_SIZE; lIdx++)
	{
		GenerateMixedI2SSample(maBufferA[lIdx]);
	}
	for(int lIdx = 0; lIdx < I2S_BUF_SIZE; lIdx++)
	{
		GenerateMixedI2SSample(maBufferB[lIdx]);
	}

	mBufferASelected = true;

	NRF_I2S->RXTXD.MAXCNT = I2S_BUF_SIZE;
	NRF_I2S->TXD.PTR = (uint32_t) maBufferA;
	NRF_I2S->EVENTS_TXPTRUPD = 0;

	// restart the MCK generator (a TASKS_STOP will disable the MCK generator)
	// Start transmitting I2S data
	NRF_I2S->TASKS_START = 1;
}

void I2SWavPlayer::StopPlayback()
{
	// Stop transmitting I2S data, a TASKS_STOP will disable the MCK generator
	NRF_I2S->TASKS_STOP = 1;
}

bool I2SWavPlayer::ContinuePlayback()
{
	bool lPlaybackIsDone = false;

	if (NRF_I2S->EVENTS_TXPTRUPD != 0) //It's time to update a buffer
	{
		if (mBufferASelected == true)
		{
			NRF_I2S->EVENTS_TXPTRUPD = 0; //Start consuming buffer B
			NRF_I2S->TXD.PTR = (uint32_t)maBufferA;
			memcpy(maBufferA, maMixedI2SSamples, sizeof(int32_t)*I2S_BUF_SIZE);
		}
		else
		{
			NRF_I2S->EVENTS_TXPTRUPD = 0; //Start consuming buffer A
			NRF_I2S->TXD.PTR = (uint32_t)maBufferB;
			memcpy(maBufferB, maMixedI2SSamples, sizeof(int32_t)*I2S_BUF_SIZE);
		}

		PopulateMixingBuffer(); //Pre-mix next set of samples

		//Toggle buffer selector
		mBufferASelected = !mBufferASelected;
	}

	if(0 == mSamplesMixed)
	{
		lPlaybackIsDone = true;
	}

	return lPlaybackIsDone;
}

bool I2SWavPlayer::IsEnded()
{
	bool lIsEnded = true;

	for(int lIdx = 0; lIdx < MAX_WAV_FILES && lIsEnded; lIdx++)
	{
		if(mapWavFile[lIdx] != nullptr && !mapWavFile[lIdx]->IsEnded())
		{
			lIsEnded = false;
		}
	}

	return lIsEnded;
}

void I2SWavPlayer::SetVolume(float aVolume)
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


void I2SWavPlayer::ClipSample(int32_t& arSample)
{
	if(arSample > INT16_MAX)
	{
		arSample = INT16_MAX;
	}
	else if(arSample < INT16_MIN)
	{
		arSample = INT16_MIN;
	}
}

void I2SWavPlayer::GenerateMixedI2SSample(int32_t& arSampleOut)
{

	int lSamplesCounter = 0;

	//Fetch 16-bit samples from even-numbered files to create left channel
	int32_t lMixedSampleLeft32 = 0;
	for(int lWavFileIdx = 0; lWavFileIdx < MAX_WAV_FILES; lWavFileIdx += 2)
	{
		SDWavFile* lpCurFilePtr = mapWavFile[lWavFileIdx];
		if(lpCurFilePtr != nullptr
				&& !lpCurFilePtr->IsPaused()
				&& !lpCurFilePtr->IsEnded())
		{
			int16_t lCurSample = 0;

			//Handle down-sampling
			if(maDownsampleParams[lWavFileIdx].mIsDownsample && lpCurFilePtr->Available() >= sizeof(int32_t))
			{
				//Double-fetch to skip a sample if we have at least
				//32 bytes (2 16-bit samples)left to read
				lpCurFilePtr->Skip16BitSamples(1);
				lpCurFilePtr->Fetch16BitSamples(&lCurSample, 1);
			}
			else
			{
				//Fetch sample from current file
				lpCurFilePtr->Fetch16BitSamples(&lCurSample, 1);
			}

			//Mix new sample with already collected samples
			lMixedSampleLeft32 += (int32_t)lCurSample;

			//Keep track of how many valid samples we read
			lSamplesCounter++;

		}
	}

	//Fetch 16-bit samples from odd-numbered files to create right channel
	int32_t lMixedSampleRight32 = 0;
	for(int lWavFileIdx = 1; lWavFileIdx < MAX_WAV_FILES; lWavFileIdx += 2)
	{
		SDWavFile* lpCurFilePtr = mapWavFile[lWavFileIdx];
		if(lpCurFilePtr != nullptr
				&& !lpCurFilePtr->IsPaused()
				&& !lpCurFilePtr->IsEnded())
		{
			//Fetch sample from current file
			int16_t lCurSample = 0;
			//Handle down-sampling
			if(maDownsampleParams[lWavFileIdx].mIsDownsample)
			{
				//Double-fetch to skip a sample
				if(lpCurFilePtr->Available() >= sizeof(int32_t))
				{
					lpCurFilePtr->Fetch16BitSamples(&lCurSample, 1);
					lpCurFilePtr->Fetch16BitSamples(&lCurSample, 1);
				}
			}
			else
			{
				lpCurFilePtr->Fetch16BitSamples(&lCurSample, 1);
			}

			//Mix new sample with already collected samples
			lMixedSampleRight32 += (int32_t)lCurSample;

			//Keep track of how many valid samples we read
			lSamplesCounter++;
		}
	}
	//Clipping
	ClipSample(lMixedSampleLeft32);
	ClipSample(lMixedSampleRight32);

	//Volume control
	if(mVolume < 1.0)
	{
		lMixedSampleLeft32 *= mVolume;
		lMixedSampleRight32 *= mVolume;
	}

	//Keep track of how many files were able to get samples from
	mSamplesMixed = lSamplesCounter;

	//16-bit left and right channel samples
	int16_t lMixedSample16Left = (int16_t)lMixedSampleLeft32;
	int16_t lMixedSample16Right = (int16_t)lMixedSampleRight32;

	//Pointer so we can address 32-bit I2S word 16-bits at a time
	int16_t* lSampleOut16Ptr = (int16_t*)&arSampleOut;

	//Copy left and right channel data into output to create 32-bit I2S word
	memcpy(lSampleOut16Ptr, &lMixedSample16Left, sizeof(int16_t));
	memcpy(lSampleOut16Ptr+1, &lMixedSample16Right, sizeof(int16_t));
}

int I2SWavPlayer::PopulateMixingBuffer()
{
	for(int lIdx = 0; lIdx < I2S_BUF_SIZE; lIdx++)
	{
		GenerateMixedI2SSample(maMixedI2SSamples[lIdx]);
	}

	return 0;
}

void I2SWavPlayer::Configure_I2S()
{
	// register structure hierarchy for I2S
	// NRF_I2S is of type NRF_I2S_Type defined in nrf52.h
	// CONFIG is of type I2S_CONFIG_Type defined in nrf52.h, sub-struct of NRF_I2S_Type
	// Struct -> Element (kinda Struct.Element)

	// Position variables (_Pos) are defined in nrf52_bitfields.h

	// Enable Tx transmission
	NRF_I2S->CONFIG.TXEN = (I2S_CONFIG_TXEN_TXEN_ENABLE << I2S_CONFIG_TXEN_TXEN_Pos);

	// Enable MCK generator
	NRF_I2S->CONFIG.MCKEN = (I2S_CONFIG_MCKEN_MCKEN_ENABLE << I2S_CONFIG_MCKEN_MCKEN_Pos);

//	// set the sample rate to a value supported by the audio amp
//	// LRCLK  ONLY  supports  8kHz,  16kHz,  32kHz,  44.1kHz,  48kHz, 88.2kHz, and 96kHz frequencies.
//	// LRCLK clocks at  11.025kHz,  12kHz,  22.05kHz  and  24kHz  are  NOT supported.
//
//	// look for /* Register: I2S_CONFIG_MCKFREQ */ in nrf52_bitfields.h
//	// MCKFREQ = 4 MHz
//	//+++NRF_I2S->CONFIG.MCKFREQ =  I2S_CONFIG_MCKFREQ_MCKFREQ_32MDIV11 << I2S_CONFIG_MCKFREQ_MCKFREQ_Pos;
//	//NRF_I2S->CONFIG.MCKFREQ =  I2S_CONFIG_MCKFREQ_MCKFREQ_32MDIV23 << I2S_CONFIG_MCKFREQ_MCKFREQ_Pos;
//	NRF_I2S->CONFIG.MCKFREQ =  I2S_CONFIG_MCKFREQ_MCKFREQ_32MDIV10 << I2S_CONFIG_MCKFREQ_MCKFREQ_Pos;
//
//	// look for /* Register: I2S_CONFIG_RATIO */ in nrf52_bitfields.h
//	// set 16kHz as default value -> Ration = 256 (4MHz/256=16kHz)
//	//+++NRF_I2S->CONFIG.RATIO = I2S_CONFIG_RATIO_RATIO_128X << I2S_CONFIG_RATIO_RATIO_Pos;
//	NRF_I2S->CONFIG.RATIO = I2S_CONFIG_RATIO_RATIO_32X << I2S_CONFIG_RATIO_RATIO_Pos;

	Configure_I2S_Speed(ee2205); //Default I2S speed

	// 16/24/32-bit  resolution, the MAX98357A supports I2S timing only!
	// Master mode, 16Bit, left aligned
	NRF_I2S->CONFIG.MODE = I2S_CONFIG_MODE_MODE_MASTER << I2S_CONFIG_MODE_MODE_Pos;

	// look for /* Register: I2S_CONFIG_SWIDTH */ in nrf52_bitfields.h
	// 16 bit
	NRF_I2S->CONFIG.SWIDTH = I2S_CONFIG_SWIDTH_SWIDTH_16BIT << I2S_CONFIG_SWIDTH_SWIDTH_Pos;

	// Left-aligned (not to be mixed up with left-justified)
	NRF_I2S->CONFIG.ALIGN = I2S_CONFIG_ALIGN_ALIGN_Left << I2S_CONFIG_ALIGN_ALIGN_Pos;

	// Format I2S (i.e. not left justified)
	NRF_I2S->CONFIG.FORMAT = I2S_CONFIG_FORMAT_FORMAT_I2S << I2S_CONFIG_FORMAT_FORMAT_Pos;

	// Use mono
	NRF_I2S->CONFIG.CHANNELS = I2S_CONFIG_CHANNELS_CHANNELS_Stereo << I2S_CONFIG_CHANNELS_CHANNELS_Pos;
	//NRF_I2S->CONFIG.CHANNELS = I2S_CONFIG_CHANNELS_CHANNELS_Left << I2S_CONFIG_CHANNELS_CHANNELS_Pos;

	// configure the pins
	NRF_I2S->PSEL.MCK = (mPinMCK << I2S_PSEL_MCK_PIN_Pos);
	NRF_I2S->PSEL.SCK = (mPinBCLK << I2S_PSEL_SCK_PIN_Pos);
	NRF_I2S->PSEL.LRCK = (mPinLRCK << I2S_PSEL_LRCK_PIN_Pos);
	NRF_I2S->PSEL.SDOUT = (mPinDIN << I2S_PSEL_SDOUT_PIN_Pos);

	// Enable the I2S module using the ENABLE register
	NRF_I2S->ENABLE = 1;

	pinMode (mPinSD, OUTPUT);
	digitalWrite (mPinSD, HIGH);
}
