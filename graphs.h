#ifndef GRAPHS_H
#define GRAPHS_H

#include <SDL.h> //graphs
#include <SDL2/SDL_ttf.h> //texts
#include <math.h>
#include <stdlib.h>

#ifndef WINDOW_CONSTANTS
#define WINDOW_CONSTANTS

#define WIDTH 800
#define HEIGHT 600

#endif // WINDOW_CONSTANTS

#define GRAPH_WIDTH 600
#define GRAPH_HEIGHT 400
#define MARGIN_WIDTH 50
#define MARGIN_HEIGHT 50
#define NUM_TICKS 10 //for axes

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

void drawText(SDL_Renderer* renderer, TTF_Font* font, const char* text, int x, int y);
float dataMax(float* data, int size);
float dataMin(float* data, int size);
float maxAbs(float a, float b);
void getCoords(float coords[2], graphBoundaries boundaries);
void drawBackground(SDL_Renderer* renderer, float* data, int length, graphBoundaries boundaries, TTF_Font* font);
void drawGraph(SDL_Renderer* renderer, float* data, int length, graphBoundaries boundaries, TTF_Font* font);

#endif // GRAPHS_H