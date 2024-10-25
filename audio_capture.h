#include <portaudio.h>
#include <pthread.h>

#ifndef AUDIO_DATA
#define AUDIO_DATA

typedef struct {
    float *samples;
    int maxFrameIndex;
    int currentIndex;
} AudioData;

#endif //AUDIO_DATA

#ifndef SAMPLE_CONSTS
#define SAMPLE_CONSTS

#define SAMPLE_RATE  10000
#define SAMPLE_COUNT SAMPLE_RATE * 1
#define FRAMES_PER_BUFFER 256

#endif // SAMPLE_CONSTS

extern pthread_mutex_t globalDataLock;
extern float* orderedData;

int customAudioCallback(const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData);
void copySamplesInOrder(AudioData *data, float *orderedSamples);