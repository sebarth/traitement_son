#include "audio_capture.h"

void copySamplesInOrder(AudioData *data, float *orderedSamples) {
    int start = data->currentIndex;
    int size = data->maxFrameIndex;
    if (data->currentIndex < 0 || data->currentIndex >= size) {
        fprintf(stderr, "ERROR: Invalid currentIndex in circular buffer.\n");
        return;
    }
    pthread_mutex_lock(&globalDataLock);
    // copy first the last values (least recent)
    memcpy(orderedSamples, data->samples + data->currentIndex, (data->maxFrameIndex - data->currentIndex) * sizeof(float));
    // then copy the first values until currentIndex (because currentIndex is the most recent)
    memcpy(orderedSamples + data->maxFrameIndex - data->currentIndex, data->samples, (data->currentIndex) * sizeof(float));
    pthread_mutex_unlock(&globalDataLock);
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
            pthread_mutex_lock(&globalDataLock);
            data->samples[data->currentIndex] = 0.0f;
            data->currentIndex = (data->currentIndex + 1) % data->maxFrameIndex;
            pthread_mutex_unlock(&globalDataLock);
        }
    } else {
        for (unsigned long i = 0; i < framesPerBuffer; i++) {
            pthread_mutex_lock(&globalDataLock);
            // store the samples in the circular buffer
            if (*in > NOISE_FLOOR || *in < -NOISE_FLOOR) {
                data->samples[data->currentIndex] = *in++;
            } else{
                data->samples[data->currentIndex] = 0;
                in++;
            }
            // update the index
            data->currentIndex = (data->currentIndex + 1) % data->maxFrameIndex;
            pthread_mutex_unlock(&globalDataLock);
        }
    }
    return paContinue;
}