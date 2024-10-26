#include <fftw3.h>
#include <SDL.h>
#include <SDL2/SDL_ttf.h>
#include <unistd.h>
#include <stdbool.h>
#include "graphs.h"

#ifndef WINDOW_CONSTANTS
#define WINDOW_CONSTANTS

#define WIDTH 1000
#define HEIGHT 800

#endif // WINDOW_CONSTANTS

#ifndef SAMPLE_CONSTS
#define SAMPLE_CONSTS

#define SAMPLE_RATE 44100
#define SAMPLE_COUNT SAMPLE_RATE * 1
#define FRAMES_PER_BUFFER 256

#endif // SAMPLE_CONSTS

#ifndef WINDOWS
#define WINDOWS

extern SDL_Window* main_window;

extern SDL_Renderer* main_renderer;

extern TTF_Font* font;
extern TTF_Font* buttonFont;
extern TTF_Font* legendFont;

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

typedef struct {
    SDL_Rect rect;
    SDL_Color bgColor;
    SDL_Color hoverColor;
    SDL_Color textColor;
    char* text;
    bool isHovered;
    void (*onClick)(void* args); // kinda ugly but loopArgs is not defined yet...
} Button;

typedef struct{
    graphBoundaries* boundaries1;
    graphBoundaries* boundaries2;
    AudioData* data;
    float* orderedData;
    fftwf_complex* fft_data;
    float* spectrum;
    float* t;
    int* quit;
    pthread_mutex_t* globalDataLock;
    int currentWindow; // value is 1 or 2
    Button* button;
    SDL_Color color1;
    SDL_Color color2;
} loopArgs;

void init(graphBoundaries* boundaries1, graphBoundaries* boundaries2, Button* button);
void loop(loopArgs args);
void renderButton(SDL_Renderer* renderer, Button* button);
bool isMouseOverButton(Button* button, int mouseX, int mouseY);
void onButtonClick(void* v_args);