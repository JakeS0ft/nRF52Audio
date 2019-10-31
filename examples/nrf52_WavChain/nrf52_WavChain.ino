#include "Arduino.h"
#include <nRF52Audio.h>

#define PIN_I2S_MCK 13
#define PIN_I2S_BCLK (A2) // A4
#define PIN_I2S_LRCK (A3) // A5
#define PIN_I2S_DIN 18 // A6
#define PIN_I2S_SD  10 // 27
#define PIN_SPI_CS  11
#define PIN_SCL 16
#define PIN_SDA 15

//Plays a file, then auotmatically transitions to playing a second file in a loop.
void PlayWavFile()
{
	//Wav files to play. Change file names to match the name of
	//whatever files you are playing.
	const char* lFileName1 = "2205/pfont1/poweron3.wav";
	const char* lFileName2 = "2205/pfont1/hum.wav";

	ChainedSDWavFile* lpWavFile1 = new ChainedSDWavFile(lFileName1, lFileName2);

	//Repeat the last file
	//NOTE: Set this to false if you don't want the chain to repeat the last file
	lpWavFile1->SetLooping(true);

	//Create a new I2S Player
	I2SWavPlayer* lpPlayer = new I2SWavPlayer(PIN_I2S_MCK,
			  	  	  	  	  	  	  	  	  PIN_I2S_BCLK,
											  PIN_I2S_LRCK,
											  PIN_I2S_DIN,
											  PIN_I2S_SD);

	lpPlayer->Init();                      //Initializes I2S playback hardware

	lpPlayer->Configure_I2S_Speed(ee2205); //Set I2S clock speed (sample rate of the file)
	                                       //ee2205 = 22.05KHz
	                                       //ee4410 = 44.1KHz

	lpPlayer->SetWavFile(lpWavFile1);      //Set file object to play
	lpPlayer->SetVolume(1.0);              //set master volume, 0.0 (mute) to 1.0 (full volume)
	lpPlayer->StartPlayback();             //Begin playing the wave files

	Serial.println("Playback started.");

	unsigned long lStartTime = millis();

	//Play for 30 seconds or until we run out of data
	//NOTE: We won't ever run out of data is looping is enabled
	bool lPlaybackEnded = false;
	while(false == lPlaybackEnded && millis() - lStartTime < 30000)
	{
		lPlaybackEnded = lpPlayer->ContinuePlayback();
	}

	//Cleanup code
	Serial.println("Playback ended.");
	lpPlayer->StopPlayback();
	lpWavFile1->Close();

}

//The setup function is called once at startup of the sketch
void setup()
{
	delay(1000);
	Serial.begin(115200);

	//Initialize SD card. Make sure to do this before creating any SDWavFile objects or
	//trying to play anything or we won't be able to read the data from the SD card
	if(!SD.begin(8000000, PIN_SPI_CS))
	{
		Serial.println("SD init failed.");
		return; //Punt. We can't work without SD card
	}
	Serial.println("SD init completed.");

	PlayWavFile();
}

// The loop function is called in an endless loop
void loop()
{
//Add your repeated code here
}
