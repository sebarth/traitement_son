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
#define SAMPLE_COUNT SAMPLE_RATE * 1
#define FRAMES_PER_BUFFER 256
#define NOISE_FLOOR 1e-3
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

#endif // CONSTANTS_H