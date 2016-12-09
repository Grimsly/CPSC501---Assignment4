#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>


char chunkID[5];
int chunkSize;
char format[5];

char subChunk1ID[5];
int subChunk1Size;
short audio_Format;
short number_Channels;
int sample_Rate;
int byte_Rate;
short block_Align;
short bits_Per_Sample;

int channelSize;
char subChunk2ID[5];
int subChunk2Size;

short* data;
short* dataIR;
int number_SamplesIR;
int number_Samples;

#define PI 3.141592653589793
#define TWO_PI (2.0 * PI)
#define SWAP(a,b)  tempr=(a);(a)=(b);(b)=tempr

void four1(double data[], int nn, int isign)
{
	unsigned long n, mmax, m, j, istep, i;
	double wtemp, wr, wpr, wpi, wi, theta;
	double tempr, tempi;

	n = nn << 1;
	j = 1;

	for (i = 1; i < n; i += 2) {
		if (j > i) {
			SWAP(data[j], data[i]);
			SWAP(data[j + 1], data[i + 1]);
		}
		m = nn;
		while (m >= 2 && j > m) {
			j -= m;
			m >>= 1;
		}
		j += m;
	}

	mmax = 2;
	while (n > mmax) {
		istep = mmax << 1;
		theta = isign * (6.28318530717959 / mmax);
		wtemp = sin(0.5 * theta);
		wpr = -2.0 * wtemp * wtemp;
		wpi = sin(theta);
		wr = 1.0;
		wi = 0.0;
		for (m = 1; m < mmax; m += 2) {
			for (i = m; i <= n; i += istep) {
				j = i + mmax;
				tempr = wr * data[j] - wi * data[j + 1];
				tempi = wr * data[j + 1] + wi * data[j];
				data[j] = data[i] - tempr;
				data[j + 1] = data[i + 1] - tempi;
				data[i] += tempr;
				data[i + 1] += tempi;
			}
			wr = (wtemp = wr) * wpr - wi * wpi + wr;
			wi = wi * wpr + wtemp * wpi + wi;
		}
		mmax = istep;
	}
}

// Print the following header information for the input file
void print()
{
	chunkID[5] = '\0';
	format[5] = '\0';
	subChunk1ID[5] = '\0';
	subChunk2ID[5] = '\0';

	printf("\n============= HEADER INFO =============\n");
	printf(" chunkID:%s\n", chunkID);
	printf(" chunkSize:%d\n", chunkSize);
	printf(" format:%s\n", format);
	printf(" subChunk1ID:%s\n", subChunk1ID);
	printf(" subChunk1Size:%d\n", subChunk1Size);
	printf(" audioFormat:%d\n", audio_Format);
	printf(" numChannels:%d\n", number_Channels);
	printf(" sampleRate:%d\n", sample_Rate);
	printf(" byteRate:%d\n", byte_Rate);
	printf(" blockAlign:%d\n", block_Align);
	printf(" bitsPerSample:%d\n", bits_Per_Sample);
	printf(" subChunk2ID:%s\n", subChunk2ID);
	printf(" subChunk2Size:%d\n", subChunk2Size);
}


// Load the input wave file 
int loadWave(char* filename)
{
	FILE* in = fopen(filename, "rb");

	if (in != NULL)
	{
		printf("Reading %s...\n", filename);

		fread(chunkID, 1, 4, in);
		fread(&chunkSize, 1, 4, in);
		fread(format, 1, 4, in);

		//sub chunk 1
		fread(subChunk1ID, 1, 4, in);
		fread(&subChunk1Size, 1, 4, in);
		fread(&audio_Format, 1, 2, in);
		fread(&number_Channels, 1, 2, in);
		fread(&sample_Rate, 1, 4, in);
		fread(&byte_Rate, 1, 4, in);
		fread(&block_Align, 1, 2, in);
		fread(&bits_Per_Sample, 1, 2, in);

		//read extra bytes
		if (subChunk1Size == 18)
		{
			short empty;
			fread(&empty, 1, 2, in);
		}

		//sub chunk 2
		fread(subChunk2ID, 1, 4, in);
		fread(&subChunk2Size, 1, 4, in);

		//read data
		int bytesPerSample = bits_Per_Sample / 8;
		number_Samples = subChunk2Size / bytesPerSample;
		data = (short*)malloc(sizeof(short) * number_Samples);

		//fread(data, 1, bytesPerSample*numSamples, in);

		int i = 0;
		short sample = 0;
		// read the .wav file data into data array
		while (fread(&sample, 1, bytesPerSample, in) == bytesPerSample)
		{
			data[i++] = sample;
			sample = 0;
		}

		//close file stream
		fclose(in);
		printf("Closing %s...\n", filename);
	}
	else
	{
		printf("Can't open file\n");
		return 0;
	}
	return 1;
}



int saveWave(char* filename, double outputSignal[], int sampleCount)
{
	FILE* out = fopen(filename, "wb");

	if (out != NULL)
	{
		printf("\nWriting %s...\n", filename);

		fwrite(chunkID, 1, 4, out);
		fwrite(&chunkSize, 1, 4, out);
		fwrite(format, 1, 4, out);

		//sub chunk 1
		fwrite(subChunk1ID, 1, 4, out);
		fwrite(&subChunk1Size, 1, 4, out);
		fwrite(&audio_Format, 1, 2, out);
		fwrite(&number_Channels, 1, 2, out);
		fwrite(&sample_Rate, 1, 4, out);
		fwrite(&byte_Rate, 1, 4, out);
		fwrite(&block_Align, 1, 2, out);
		fwrite(&bits_Per_Sample, 1, 2, out);

		//read extra bytes
		if (subChunk1Size == 18)
		{
			short empty = 0;
			fwrite(&empty, 1, 2, out);
		}

		//sub chunk 2
		fwrite(subChunk2ID, 1, 4, out);
		fwrite(&subChunk2Size, 1, 4, out);

		//read data
		int bytesPerSample = bits_Per_Sample / 8;

		float maxSample = -1;
		float MAX_VAL = 32767.f;	

		for (int i = 0; i < sampleCount; i++)
		{
			if (i == 0)
			{
				maxSample = outputSignal[0];
			}
			else if (outputSignal[i] > maxSample)
			{
				maxSample = outputSignal[i];
			}
		}

		//scale and re write the data
		for (int i = 0; i<sampleCount; ++i)
		{
			outputSignal[i] = (outputSignal[i] / maxSample);
			short sample = (short)(outputSignal[i] * MAX_VAL);
			fwrite(&sample, 1, bytesPerSample, out);
		}

		//clean up
		free(outputSignal);
		fclose(out);
		printf("Closing %s...\n", filename);
	}
	else
	{
		printf("Can't open file\n");
		return 0;
	}
	return 1;
}

int loadIR(char* filename)
{
	FILE* in = fopen(filename, "rb");
	int subChunk2SizeIR;


	if (in != NULL)
	{
printf("Reading %s...\n", filename);

//read subchunk1size
int IRsubChunk1Size;
fseek(in, 16, SEEK_SET);
fread(&IRsubChunk1Size, 1, 4, in);
if (IRsubChunk1Size == 18)
{
	short empty;
	fread(&empty, 1, 2, in);
}

//bits per sample
short bits_Per_SampleIR;
fseek(in, 34, SEEK_SET);
fread(&bits_Per_SampleIR, 1, 2, in);

//subchunk2size
if (IRsubChunk1Size == 18)
{
	fseek(in, 42, SEEK_SET);
	fread(&subChunk2SizeIR, 1, 4, in);
	fseek(in, 46, SEEK_SET);
}
else
{
	fseek(in, 40, SEEK_SET);
	fread(&subChunk2SizeIR, 1, 4, in);
	fseek(in, 44, SEEK_SET);
}

int bytesPerSampleIR = bits_Per_SampleIR / 8;
number_SamplesIR = subChunk2SizeIR / bytesPerSampleIR;

dataIR = (short*)malloc(sizeof(short) * number_SamplesIR);

int i = 0;
short sample = 0;
while (fread(&sample, 1, bytesPerSampleIR, in) == bytesPerSampleIR)
{
	dataIR[i++] = sample;
	sample = 0;
}
// close file input stream
fclose(in);
	}
	else
	{
		printf("Can't open file\n");
		return 0;
	}
	return 1;
}



int main(int argc, char* argv[])
{
	clock_t begin, end;
	double time_spent;
	if (argc != 4)
	{
		printf("Incorrect number of arguments");
		exit(-1);
	}
	begin = clock();
	char* inputFile = argv[1];
	if (loadWave(inputFile))
		print();

	char* IRfile = argv[2];
	loadIR(IRfile);

	int maxSize;
	// find the max size out of input and ir
	if (number_Samples > number_SamplesIR)
		maxSize = number_Samples;
	else
		maxSize = number_SamplesIR;

	// used to make sure that the max size of array is a power of 2
	int maxSizePow2;
	maxSizePow2 = (int)log2(maxSize) + 1;
	maxSizePow2 = pow(2, maxSizePow2);
	int doubleMaxSize = 2 * maxSizePow2;

	//allocate memory for complex input and IR using double max size
	double *complexInput;
	double *complexIR;
	complexInput = (double*)malloc(sizeof(double) * doubleMaxSize);
	complexIR = (double*)malloc(sizeof(double) * doubleMaxSize);


	// pad the array with 0's
	for (int i = 0; i < doubleMaxSize; i++)
	{
		complexIR[i] = 0.0;
	}

	for (int i = 0; i < doubleMaxSize; i++)
	{
		complexInput[i] = 0.0;
	}

	double MAX_VAL = 32767.f;

	// real numbers
	for (int i = 0; i < number_Samples; i++)
	{
		complexInput[2 * i] = ((double)data[i]) / 32767.0;
	}

	// real numbers
	for (int i = 0; i < number_SamplesIR; i++)
	{
		complexIR[2 * i] = ((double)dataIR[i]) / 32767.0;
	}


	four1(complexInput, maxSizePow2, 1);
	four1(complexIR, maxSizePow2, 1);

	//allocate space for the output signal
	double *complexOutput;
	complexOutput = (double*)malloc(sizeof(double) *doubleMaxSize);

	// complex multiplication of the input and ir response 
	for (int i = 0; i < maxSizePow2; i++)
	{
		complexOutput[i * 2] = complexInput[i] * complexIR[i] - complexInput[i + 1] * complexIR[i + 1];
	}

	for (int i = 0; i < maxSizePow2; i++)
	{
		complexOutput[i * 2 + 1] = complexInput[i + 1] * complexIR[i] + complexInput[i] * complexIR[i + 1];
	}


	char* outputFile = argv[3];

	four1(complexOutput, maxSizePow2, -1);
	saveWave(outputFile, complexOutput, maxSizePow2);

	end = clock();
	time_spent = (double)(end - begin);
	printf("Time spent: %f seconds", (end - begin) / (double)CLOCKS_PER_SEC);
	free(data);
	free(dataIR);
	free(complexIR);
	free(complexInput);
}