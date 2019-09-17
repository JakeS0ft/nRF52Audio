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

//Plays a single wave file until it ends
void PlayWavFiles()
{
	//Wave files to play. Change "22CANT1.WAV" and other file names
	//to match the name of whatever files you are playing.
//	SDWavFile* lpWavFile1 = new SDWavFile("2205/swing1.wav");
//	SDWavFile* lpWavFile2 = new SDWavFile("2205/font.wav");
//	SDWavFile* lpWavFile3 = new SDWavFile("2205/hum.wav");
//	SDWavFile* lpWavFile4 = new SDWavFile("2205/lockup.wav");
//	SDWavFile* lpWavFile5 = new SDWavFile("2205/CANT2.WAV");

	SDWavFile* lpWavFile0 = new SDWavFile("2205/font.wav");
	SDWavFile* lpWavFile1 = new SDWavFile("2205/hum.wav");
	SDWavFile* lpWavFile2 = new SDWavFile("2205/CANT2.WAV");

	//Set files to loop
	lpWavFile0->SetLooping(true);
	lpWavFile1->SetLooping(true);
	lpWavFile2->SetLooping(true);

	//Set relative volumes for each file (1.0 = full, 0.0 = mute)
	lpWavFile0->SetVolume(0.0);
	lpWavFile1->SetVolume(1.0);
	lpWavFile2->SetVolume(0.7);

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

	lpPlayer->SetWavFile(lpWavFile0, 0);   //Set file object to play for channel 0
	lpPlayer->SetWavFile(lpWavFile1, 1);   //Set file object to play for channel 0
	lpPlayer->SetWavFile(lpWavFile2, 2);   //Set file object to play for channel 0

	lpPlayer->SetVolume(0.15);              //set master volume, 0.0 (mute) to 1.0 (full volume)
	lpPlayer->StartPlayback();             //Begin playing the wave files all at once

	Serial.println("Playback started.");
	unsigned long lStartTime = millis(); //Keep track of when we started playback

	//Keep playing as long as playback hasn't ended
	while(false == lpPlayer->ContinuePlayback() && millis() - lStartTime < 60000)
	{
		//Remove the channel 0 after 10 seconds
		if(millis() - lStartTime > 10000)
		{
			lpPlayer->SetWavFile(nullptr, 0);
		}

		//Remove channel 1 after 20 seconds
		if(millis() - lStartTime > 20000)
		{
			lpPlayer->SetWavFile(nullptr, 1);
		}

		//Remove channel 2 after 30 seconds
		if(millis() - lStartTime > 30000)
		{
			lpPlayer->SetWavFile(nullptr, 2);
		}

	}

	//Cleanup code
	Serial.println("Playback ended.");
	lpPlayer->StopPlayback();
	lpWavFile0->Close();

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

	PlayWavFiles();
}

// The loop function is called in an endless loop
void loop()
{
//Add your repeated code here
}
