#include <stdio.h>
#include <stdlib.h>
#include <portaudio.h>
#include <string.h>

#define SAMPLE_RATE  44100
#define FRAMES_PER_BUFFER 256

typedef struct {
    float *samples;
    int maxFrameIndex;
    int currentIndex;
} AudioData;

static int audioCallback(const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData) {
    AudioData *data = (AudioData*)userData;
    const float *in = (const float*)inputBuffer;

    if (inputBuffer == NULL) {
        for (unsigned long i = 0; i < framesPerBuffer; i++) {
            printf("No Input\n");
        }
    } else {
        for (unsigned long i = 0; i < framesPerBuffer; i++) {
            // store the samples in the circular buffer
            data->samples[data->currentIndex] = *in++;
            // update the index
            data->currentIndex = (data->currentIndex + 1) % data->maxFrameIndex;
        }
    }
    return paContinue;
}

void copySamplesInOrder(AudioData *data, float *orderedSamples) {
    int start = data->currentIndex;
    int size = data->maxFrameIndex;
    // copy first the last values (least recent)
    memcpy(orderedSamples, data->samples + data->currentIndex, (data->maxFrameIndex - data->currentIndex) * sizeof(double));
    // then copy the first values until currentIndex (because currentIndex is the most recent)
    memcpy(orderedSamples + data->maxFrameIndex - data->currentIndex, data->samples, (data->currentIndex) * sizeof(double));
}

int main(void) {
    PaStream *stream;
    PaError err;
    AudioData data;

    data.maxFrameIndex = SAMPLE_RATE * 5; // 5 seconds
    data.samples = (float*)malloc(sizeof(float) * data.maxFrameIndex);
    data.currentIndex = 0;

    // temprary buffer to store the samples in the right order
    float *orderedSamples = (float*)malloc(sizeof(float) * data.maxFrameIndex);

    err = Pa_Initialize();
    if (err != paNoError) goto error;

    err = Pa_OpenDefaultStream(&stream,
                               1,          // mono input
                               0,          // no output
                               paFloat32,  // format 32 bits float
                               SAMPLE_RATE,
                               FRAMES_PER_BUFFER,
                               audioCallback,
                               &data);
    if (err != paNoError) goto error;

    err = Pa_StartStream(stream);
    if (err != paNoError) goto error;    
    // TODO add code here before stopping the stream

    err = Pa_StopStream(stream);
    if (err != paNoError) goto error;

    err = Pa_CloseStream(stream);
    if (err != paNoError) goto error;

    Pa_Terminate();

    free(data.samples);
    free(orderedSamples);
    return 0;

error:
    Pa_Terminate();
    fprintf(stderr, "Une erreur est survenue: %s\n", Pa_GetErrorText(err));
    return -1;
}
