#include <stdio.h>
#include <unistd.h>

#include <faac.h>
#include <portaudio.h>

#define PA_FRAMES 2048
#define MAX_SAMPLES 100

typedef struct
{
    char *name;
    int dev_id;
    int sr_list[10]; //8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000, 0

}snd_dev_t;

unsigned char* pbAACBuffer;
int samplerate = 44100;
int channel = 2;
int total_samples = 0;
unsigned long nInputSamples = 0;
unsigned long nMaxOutputBytes = 0;

FILE* fp_out; // AAC file for output
faacEncHandle hEncoder;

PaStream *stream;

//this function is called by PortAudio when new audio data arrived
int snd_callback(const void *input,
                 void *output,
                 unsigned long frameCount,
                 const PaStreamCallbackTimeInfo* timeInfo,
                 PaStreamCallbackFlags statusFlags,
                 void *userData)
{
		printf("%d: framecount: %ld\n", total_samples, frameCount);
	
    int nRet = faacEncEncode(
        hEncoder, (int*) input, frameCount * channel, pbAACBuffer, nMaxOutputBytes);

    fwrite(pbAACBuffer, 1, nRet, fp_out);
    
    total_samples++;

    return 0;
}

int snd_init()
{
    char info_buf[256];

    PaError p_err;
    if((p_err = Pa_Initialize()) != paNoError)
    {
        printf("PortAudio init failed:\n%s\n", Pa_GetErrorText(p_err));

        return 1;
    }

    fclose(fp_out);
    fp_out = fopen("./out_live.aac", "wb");
    
    return 0;
}

int snd_open_stream()
{
	  faacEncConfigurationPtr pConfiguration; 
    hEncoder = faacEncOpen(samplerate, channel, &nInputSamples, &nMaxOutputBytes);
    if(hEncoder == NULL)
    {
        printf("[ERROR] Failed to call faacEncOpen()\n");
        return -1;
    }

    // (2.1) Get current encoding configuration
    pConfiguration = faacEncGetCurrentConfiguration(hEncoder);
    pConfiguration->inputFormat = FAAC_INPUT_16BIT;

    // (2.2) Set encoding configuration
    int nRet = faacEncSetConfiguration(hEncoder, pConfiguration);
    
    pbAACBuffer = new unsigned char [nMaxOutputBytes];


		////////////////////////
    char info_buf[256];

    PaStreamParameters pa_params;
    PaError pa_err;
    
    pa_params.device = Pa_GetDefaultInputDevice(); /* default input device */
    if (pa_params.device == paNoDevice) {
        fprintf(stderr,"Error: No default input device.\n");
        return 1;
    }
    pa_params.channelCount = channel;
    pa_params.sampleFormat = paInt16;
    pa_params.suggestedLatency = Pa_GetDeviceInfo(pa_params.device)->defaultHighInputLatency;
    pa_params.hostApiSpecificStreamInfo = NULL;

    pa_err = Pa_IsFormatSupported(&pa_params, NULL, samplerate);
    if(pa_err != paFormatIsSupported)
    {
    	  printf("Samplerate not supported: %dHz\n", samplerate);
        return 1;
    }

    pa_err = Pa_OpenStream(&stream, &pa_params, NULL,
                            samplerate, PA_FRAMES,
                            paNoFlag, snd_callback, NULL);

    if(pa_err != paNoError)
    {
        printf("error opening sound device: \n%s\n", Pa_GetErrorText(pa_err));
        return 1;
    }
    
    Pa_StartStream(stream);
    return 0;
}

void snd_close()
{
    fclose(fp_out);

    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();    
}

int main(int argc, char* argv[])
{
		snd_init();
		snd_open_stream();
		
		while(total_samples < MAX_SAMPLES) 
		{
				Pa_Sleep(1000);
		}
		
		snd_close();
		
		return 0;
}	
