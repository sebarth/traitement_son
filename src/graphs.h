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
void getCoords(float coords[2], graphBoundaries boundaries);
void drawBackground(SDL_Renderer* renderer, float* data, int length, graphBoundaries boundaries, TTF_Font* font);
void drawLegend(SDL_Renderer* renderer, TTF_Font* font, char* legendx, char* legendy);
void drawGraph(SDL_Renderer* renderer, float* data, int length, graphBoundaries boundaries, TTF_Font* font, TTF_Font* legend_font, char* legendx, char* legendy);

#endif // GRAPHS_H