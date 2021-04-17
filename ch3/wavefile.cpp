/*===================================================

wavefile.h

This handles reading and writing wave files

Written by Alan Wolfe 5/2012

http://demofox.org

some useful links about the wave file format:
http://www.piclist.com/techref/io/serial/midi/wave.html
https://ccrma.stanford.edu/courses/422/projects/WaveFormat/
http://www-mmsp.ece.mcgill.ca/documents/audioformats/wave/wave.html

Note: This source code assumes that you are on a little endian machine.

===================================================*/

#include "wavefile.h"
#include <stdio.h>
#include <memory.h>
#include <string.h>

//this struct is the minimal required header data for a wav file
struct SMinimalWaveFileHeader
{
	//the main chunk
	unsigned char m_szChunkID[4];      //0
	uint32		  m_nChunkSize;        //4
	unsigned char m_szFormat[4];       //8

	//sub chunk 1 "fmt "
	unsigned char m_szSubChunk1ID[4];  //12
	uint32		  m_nSubChunk1Size;    //16
	uint16		  m_nAudioFormat;      //18
	uint16		  m_nNumChannels;      //20
	uint32		  m_nSampleRate;       //24
	uint32		  m_nByteRate;         //28
	uint16		  m_nBlockAlign;       //30
	uint16		  m_nBitsPerSample;    //32

	//sub chunk 2 "data"
	unsigned char m_szSubChunk2ID[4];  //36
	uint32		  m_nSubChunk2Size;    //40

	//then comes the data!
};

//0 to 255
void ConvertFloatToAudioSample(float fFloat, uint8 &nOut)
{
	fFloat = (fFloat + 1.0f) * 127.5f;
	CLAMP(fFloat,0.0f,255.0f);
	nOut = (uint8)fFloat;
}

//–32,768 to 32,767
void ConvertFloatToAudioSample(float fFloat, int16 &nOut)
{
	fFloat *= 32767.0f;
	CLAMP(fFloat,-32768.0f,32767.0f);
	nOut = (int16)fFloat;
}

//–2,147,483,648 to 2,147,483,647
void ConvertFloatToAudioSample(float fFloat, int32 &nOut)
{
	double dDouble = (double)fFloat;
	dDouble *= 2147483647.0;
	CLAMP(dDouble,-2147483648.0,2147483647.0);
	nOut = (int32)dDouble;
}

//this writes a wave file
//specify sample bit count as the template parameter
//can be uint8, int16 or int32
template <typename T>
bool WriteWaveFile(const char *szFileName, float *pRawData, int32 nNumSamples, int16 nNumChannels, int32 nSampleRate, bool bNormalizeData /*= true*/)
{
	float *pOrigRawData = pRawData;

	//open the file if we can
	FILE *File = fopen(szFileName,"w+b");
	if(!File)
	{
		return false;
	}

	//normalize the audio data if we should
	if(bNormalizeData)
	{
		pRawData = new float[nNumSamples];
		memcpy(pRawData,pOrigRawData,sizeof(float)*nNumSamples);
		NormalizeAudioData(pRawData,nNumSamples);
	}

	//calculate bits per sample and the data size
	int32 nBitsPerSample = sizeof(T) * 8;
	int nDataSize = nNumSamples * sizeof(T);

	SMinimalWaveFileHeader waveHeader;

	//fill out the main chunk
	memcpy(waveHeader.m_szChunkID,"RIFF",4);
	waveHeader.m_nChunkSize = nDataSize + 36;
	memcpy(waveHeader.m_szFormat,"WAVE",4);

	//fill out sub chunk 1 "fmt "
	memcpy(waveHeader.m_szSubChunk1ID,"fmt ",4);
	waveHeader.m_nSubChunk1Size = 16;
	waveHeader.m_nAudioFormat = 1;
	waveHeader.m_nNumChannels = nNumChannels;
	waveHeader.m_nSampleRate = nSampleRate;
	waveHeader.m_nByteRate = nSampleRate * nNumChannels * nBitsPerSample / 8;
	waveHeader.m_nBlockAlign = nNumChannels * nBitsPerSample / 8;
	waveHeader.m_nBitsPerSample = nBitsPerSample;

	//fill out sub chunk 2 "data"
	memcpy(waveHeader.m_szSubChunk2ID,"data",4);
	waveHeader.m_nSubChunk2Size = nDataSize;

	//write the header
	fwrite(&waveHeader,sizeof(SMinimalWaveFileHeader),1,File);

	//write the wave data itself, converting it from float to the type specified
	T *pData = new T[nNumSamples];
	for(int nIndex = 0; nIndex < nNumSamples; ++nIndex)
		ConvertFloatToAudioSample(pRawData[nIndex],pData[nIndex]);
	fwrite(pData,nDataSize,1,File);
	delete[] pData;

	//if we normalized the data-, free the buffer we allocated
	if(bNormalizeData)
	{
		delete[] pRawData;
	}

	//close the file and return success
	fclose(File);
	return true;
}

//explicit types to get around the templatized function compilation problem
bool WriteWaveFileUint8(const char *szFileName, float *pRawData, int32 nNumSamples, int16 nNumChannels, int32 nSampleRate, bool bNormalizeData /*= true*/)
{
	return WriteWaveFile<uint8>(szFileName,pRawData,nNumSamples,nNumChannels,nSampleRate,bNormalizeData);
}

bool WriteWaveFileInt16(const char *szFileName, float *pRawData, int32 nNumSamples, int16 nNumChannels, int32 nSampleRate, bool bNormalizeData /*= true*/)
{
	return WriteWaveFile<int16>(szFileName,pRawData,nNumSamples,nNumChannels,nSampleRate,bNormalizeData);
}

bool WriteWaveFileInt32(const char *szFileName, float *pRawData, int32 nNumSamples, int16 nNumChannels, int32 nSampleRate, bool bNormalizeData /*= true*/)
{
	return WriteWaveFile<int32>(szFileName,pRawData,nNumSamples,nNumChannels,nSampleRate,bNormalizeData);
}

float PCMToFloat(unsigned char *pPCMData, int nNumBytes)
{
	int32 nData = 0;
	unsigned char *pData = (unsigned char *)&nData;

	switch(nNumBytes)
	{
		case 1:
		{
			pData[3] = pPCMData[0];
			return ((float)nData) / ((float)0x000000ff);
		}
		case 2:
		{
			pData[2] = pPCMData[0];
			pData[3] = pPCMData[1];
			return ((float)nData) / ((float)0x00007fff);
		}
		case 3:
		{
			pData[1] = pPCMData[0];
			pData[2] = pPCMData[1];
			pData[3] = pPCMData[2];
			return ((float)nData) / ((float)0x007fffff);
		}
		case 4:
		{
			pData[0] = pPCMData[0];
			pData[1] = pPCMData[1];
			pData[2] = pPCMData[2];
			pData[3] = pPCMData[3];
			return ((float)nData) / ((float)0x7fffffff);
		}
		default:
		{
			return 0.0f;
		}
	}
}

//loads a wave file in.  Converts from source format into the specified number of channels and sample rate.
//normalizes the data by default.
//returns the data as floats and returns the number of samples in nNumSamples
bool ReadWaveFile(const char *szFileName, float *&pData, int32 &nNumSamples, int16 nNumChannels, int32 nSampleRate, bool bNormalizeData /*= true*/)
{
	//open the file if we can
	FILE *File = fopen(szFileName,"rb");
	if(!File)
	{
		return false;
	}

	//read the main chunk ID and make sure it's "RIFF"
	char buffer[5];
	buffer[4] = 0;
	if(fread(buffer,4,1,File) != 1 || strcmp(buffer,"RIFF"))
	{
		fclose(File);
		return false;
	}

	//read the main chunk size
	uint32 nChunkSize;
	if(fread(&nChunkSize,4,1,File) != 1)
	{
		fclose(File);
		return false;
	}

	//read the format and make sure it's "WAVE"
	if(fread(buffer,4,1,File) != 1 || strcmp(buffer,"WAVE"))
	{
		fclose(File);
		return false;
	}

	long chunkPosFmt = -1;
	long chunkPosData = -1;

	while(chunkPosFmt == -1 || chunkPosData == -1)
	{
		//read a sub chunk id and a chunk size if we can
		if(fread(buffer,4,1,File) != 1 || fread(&nChunkSize,4,1,File) != 1)
		{
			fclose(File);
			return false;
		}

		//if we hit a fmt
		if(!strcmp(buffer,"fmt "))
		{
			chunkPosFmt = ftell(File) - 8;
		}
		//else if we hit a data
		else if(!strcmp(buffer,"data"))
		{
			chunkPosData = ftell(File) - 8;
		}

		//skip to the next chunk
		fseek(File,nChunkSize,SEEK_CUR);
	}

	//we'll use this handy struct to load in 
	SMinimalWaveFileHeader waveData;

	//load the fmt part if we can
	fseek(File,chunkPosFmt,SEEK_SET);
	if(fread(&waveData.m_szSubChunk1ID,24,1,File) != 1)
	{
		fclose(File);
		return false;
	}

	//load the data part if we can
	fseek(File,chunkPosData,SEEK_SET);
	if(fread(&waveData.m_szSubChunk2ID,8,1,File) != 1)
	{
		fclose(File);
		return false;
	}

	//verify a couple things about the file data
	if(waveData.m_nAudioFormat != 1 ||       //only pcm data
	   waveData.m_nNumChannels < 1 ||        //must have a channel
	   waveData.m_nNumChannels > 2 ||        //must not have more than 2
	   waveData.m_nBitsPerSample > 32 ||     //32 bits per sample max
	   waveData.m_nBitsPerSample % 8 != 0 || //must be a multiple of 8 bites
	   waveData.m_nBlockAlign > 8)           //blocks must be 8 bytes or lower
	{
		fclose(File);
		return false;
	}

	//figure out how many samples and blocks there are total in the source data
	int nBytesPerBlock = waveData.m_nBlockAlign;
	int nNumBlocks = waveData.m_nSubChunk2Size / nBytesPerBlock;
	int nNumSourceSamples = nNumBlocks * waveData.m_nNumChannels;

	//allocate space for the source samples
	float *pSourceSamples = new float[nNumSourceSamples];

	//maximum size of a block is 8 bytes.  4 bytes per samples, 2 channels
	unsigned char pBlockData[8];
	memset(pBlockData,0,8);

	//read in the source samples at whatever sample rate / number of channels it might be in
	int nBytesPerSample = nBytesPerBlock / waveData.m_nNumChannels;
	for(int nIndex = 0; nIndex < nNumSourceSamples; nIndex += waveData.m_nNumChannels)
	{
		//read in a block
		if(fread(pBlockData,waveData.m_nBlockAlign,1,File) != 1)
		{
			delete[] pSourceSamples;
			fclose(File);
			return false;
		}

		//get the first sample
		pSourceSamples[nIndex] = PCMToFloat(pBlockData,nBytesPerSample);

		//get the second sample if there is one
		if(waveData.m_nNumChannels == 2)
		{
			pSourceSamples[nIndex+1] = PCMToFloat(&pBlockData[nBytesPerSample],nBytesPerSample);
		}
	}

	//re-sample the sample rate up or down as needed
	ResampleData(pSourceSamples, nNumSourceSamples, waveData.m_nSampleRate, nSampleRate);

	//handle switching from mono to stereo or vice versa
	ChangeNumChannels(pSourceSamples, nNumSourceSamples, waveData.m_nNumChannels, nNumChannels);

	//normalize the data if we should
	if(bNormalizeData)
	{
		NormalizeAudioData(pSourceSamples,nNumSourceSamples);
	}

	//return our data
	pData = pSourceSamples;
	nNumSamples = nNumSourceSamples;

	return true;
}