#ifndef AUDIO_CAPTURE_H
#define AUDIO_CAPTURE_H

#include "libs.h"
#include "cross_platform.h"
#include "constants.h"

extern pthread_mutex_t globalDataLock;
extern float* orderedData;

int customAudioCallback(const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData);
void copySamplesInOrder(AudioData *data, float *orderedSamples);

#endif // AUDIO_CAPTURE_H