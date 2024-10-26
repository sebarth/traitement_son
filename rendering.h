#ifndef RENDERING_H
#define RENDERING_H
#include <fftw3.h>
#include <SDL.h>
#include <SDL2/SDL_ttf.h>
#include "graphs.h"

#ifndef WINDOW_CONSTANTS
#define WINDOW_CONSTANTS

#define WIDTH 1000
#define HEIGHT 800

#endif // WINDOW_CONSTANTS

#ifndef SAMPLE_CONSTS
#define SAMPLE_CONSTS

#define SAMPLE_RATE 10000
#define SAMPLE_COUNT SAMPLE_RATE * 1
#define FRAMES_PER_BUFFER 1

#endif // SAMPLE_CONSTS

#ifndef WINDOWS
#define WINDOWS

extern SDL_Window* window1;
extern SDL_Window* window2;

extern SDL_Renderer* renderer1;
extern SDL_Renderer* renderer2;

extern TTF_Font* font;

#endif // WINDOWS

#ifndef AUDIO_DATA
#define AUDIO_DATA

typedef struct {
    float *samples;
    int maxFrameIndex;
    int currentIndex;
} AudioData;

#endif // AUDIO_DATA

#ifndef GRAPH_BOUNDARIES
#define GRAPH_BOUNDARIES

typedef struct {
    float min;
    float max;
} Interval;

typedef struct {
    Interval xInterval;
    Interval yInterval;
} graphBoundaries;

#endif // GRAPH_BOUNDARIES

typedef struct{
    graphBoundaries* boundaries1;
    graphBoundaries* boundaries2;
    AudioData* data;
    float* orderedData;
    fftwf_complex* fft_data;
    float* spectrum;
    float* t;
    int* quit1;
    int* quit2;
    pthread_mutex_t* globalDataLock;
} loopArgs;

void init(graphBoundaries* boundaries1, graphBoundaries* boundaries2);
void loop(loopArgs args);

#endif // RENDERING_H