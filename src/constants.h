#ifndef CONSTANTS_H
#define CONSTANTS_H

// AudioData struct
typedef struct {
    float *samples;
    int maxFrameIndex;
    int currentIndex;
} AudioData;

// Audio related constants

#define SAMPLE_RATE  44100
#define SAMPLE_COUNT (int) (SAMPLE_RATE / 25)
#define FRAMES_PER_BUFFER 256
#define NOISE_FLOOR 0.0f
#define LOG_NOISE_FLOOR -3.0f

// Graph boundaries
typedef struct {
    float min;
    float max;
} Interval;

typedef struct {
    Interval xInterval;
    Interval yInterval;
} graphBoundaries;

// Windows
#ifndef SDL_LIBS
#define SDL_LIBS

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_gfx.h>

#endif // SDL_LIBS

extern SDL_Window* main_window;

extern SDL_Renderer* main_renderer;

// Window constants
#define WIDTH 1440
#define HEIGHT 800

// Handle different views
typedef enum {
    VIEW_INPUT = 0,
    VIEW_SPECTRUM = 1<<0,
    VIEW_AUTOCORRELATION = 1<<1,
    VIEW_VOWEL_PREDICTION = VIEW_SPECTRUM | VIEW_AUTOCORRELATION
} ViewType;


#endif // CONSTANTS_H