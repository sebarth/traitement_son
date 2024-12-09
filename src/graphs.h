#ifndef GRAPHS_H
#define GRAPHS_H

#include "libs.h"
#include "constants.h"

#define MARGIN_WIDTH 150
#define MARGIN_HEIGHT 150
#define NUM_TICKS 10 //for axes

void drawText(SDL_Renderer* renderer, TTF_Font* font, const char* text, int x, int y, SDL_Color color);
float dataMax(float* data, int size);
float dataMin(float* data, int size);
void drawGraph(SDL_Renderer* renderer, float* data, SDL_Color line_color, int length, graphBoundaries boundaries, TTF_Font* font, TTF_Font* title_font, TTF_Font* legend_font, char* legendx, char* legendy, char* title);
void drawPeaks(SDL_Renderer* renderer, int* peaks, int peak_count, float* data, int data_size, graphBoundaries boundaries, SDL_Color peak_color);

#endif // GRAPHS_H