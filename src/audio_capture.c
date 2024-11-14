#include "audio_capture.h"

void copySamplesInOrder(AudioData *data, float *orderedSamples) {
    int start = data->currentIndex;
    int size = data->maxFrameIndex;
    // copy first the last values (least recent)
    memcpy(orderedSamples, data->samples + data->currentIndex, (data->maxFrameIndex - data->currentIndex) * sizeof(double));
    // then copy the first values until currentIndex (because currentIndex is the most recent)
    memcpy(orderedSamples + data->maxFrameIndex - data->currentIndex, data->samples, (data->currentIndex) * sizeof(double));
}

int customAudioCallback(const void *inputBuffer, void *outputBuffer,
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
            pthread_mutex_lock(&globalDataLock);
            // store the samples in the circular buffer
            data->samples[data->currentIndex] = *in++;
            // update the index
            data->currentIndex = (data->currentIndex + 1) % data->maxFrameIndex;
            pthread_mutex_unlock(&globalDataLock);
        }
    }
    return paContinue;
}