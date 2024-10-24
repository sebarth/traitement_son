#ifndef SIGNAL_GENERATION_H
#define SIGNAL_GENERATION_H

#include <fftw3.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "fft.h"

#ifndef SAMPLE_CONSTS
#define SAMPLE_CONSTS

#define SAMPLE_COUNT 1024
#define SAMPLE_RATE 10000
#define FRAMES_PER_BUFFER 1

#endif // SAMPLE_CONSTS

#ifndef AUDIO_DATA
#define AUDIO_DATA

typedef struct {
    float *samples;
    int maxFrameIndex;
    int currentIndex;
} AudioData;

#endif // AUDIO_DATA


typedef struct{
    AudioData* data;
    float* orderedData;
    float* t;
    fftwf_complex* fft_data;
    float* spectrum;
    int* quit1;
    int* quit2;
    pthread_mutex_t* globalDataLock;
    fftwf_complex* freq_domain;
    float* time_domain;
    fftwf_plan fft_plan;
} updateArgs;

void signal_init(fftwf_complex* freq_domain, float* time_domain, float freq);
float signal(float t, float* time_domain);
void updateData(updateArgs args);
void orderData(AudioData data, float* orderedData);
void* updateDataPtr(void* update_args);

#endif // SIGNAL_GENERATION_H