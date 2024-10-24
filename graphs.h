#include <SDL2/SDL.h> //graphs
#include <SDL2/SDL_ttf.h> //texts
#include <math.h>
#include <stdlib.h>

const int WIDTH = 1000;
const int HEIGHT = 800;
const int GRAPH_WIDTH = 800;
const int GRAPH_HEIGHT = 600;
const int MARGIN_WIDTH = 100;
const int MARGIN_HEIGHT = 100;
const int NUM_TICKS = 10; //for axes

typedef struct {
    float min;
    float max;
} Interval;
typedef struct {
    Interval xInterval;
    Interval yInterval;
} graphBoundaries;

void drawText(SDL_Renderer* renderer, TTF_Font* font, const char* text, int x, int y) {
    SDL_Color black = {0, 0, 0};
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, black);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    int texW = 0;
    int texH = 0;
    SDL_QueryTexture(texture, NULL, NULL, &texW, &texH);
    SDL_Rect dstrect = {x, y, texW, texH};

    SDL_RenderCopy(renderer, texture, NULL, &dstrect);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

float dataMax(float* data, int size) {
    float max = FLT_MIN;

    for (int i = 0; i < size; i++) {
        if (data[i] > max) max = data[i];
    }

    return max;
}
float dataMin(float* data, int size) {
    float min = FLT_MAX;

    for (int i = 0; i < size; i++) {
        if (data[i] < min) min = data[i];
    }

    return min;
}

float maxAbs(float a, float b){
    // returns the max absolute value
    return (abs(a) > abs(b)) ? abs(a) : abs(b);
}

void getCoords(float coords[2], graphBoundaries boundaries){
    int graph_width = WIDTH - 2*MARGIN_WIDTH;
    int graph_margin_width = (graph_width) / 20;
    graph_width -= 2*graph_margin_width;
    int graph_height = HEIGHT - 2*MARGIN_HEIGHT;
    int graph_margin_height = (graph_height) / 20;
    graph_height -= 2*graph_margin_height;

    // f(x) = ax+b
    // x1 = Interval.max, x2 = Interval.min, y1 = f(x1), y2 = f(x2)

    // a = (y1-y2)/(x1-x2)
    float a_x = ((float)(graph_width)) / (boundaries.xInterval.max - boundaries.xInterval.min);
    // b = y1 - x1*a
    float b_x = (float) (MARGIN_WIDTH + graph_width + graph_margin_width) -  boundaries.xInterval.max * a_x;
    coords[0] = round(a_x * coords[0] + b_x);
    
    //do the same for y
    float a_y = (float) (graph_height) / (boundaries.yInterval.max - boundaries.yInterval.min);
    float b_y = (float) (MARGIN_HEIGHT + graph_height + graph_margin_height) -  boundaries.yInterval.max * a_y;
    coords[1] = HEIGHT - round(a_y * coords[1] + b_y);

    return;
}

void drawBackground(SDL_Renderer* renderer, float* data, int length, graphBoundaries boundaries, TTF_Font* font){
    //draw axes
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawLine(renderer, MARGIN_WIDTH, MARGIN_HEIGHT, WIDTH - MARGIN_WIDTH, MARGIN_HEIGHT);
    SDL_RenderDrawLine(renderer, MARGIN_WIDTH, HEIGHT - MARGIN_HEIGHT, WIDTH - MARGIN_WIDTH, HEIGHT - MARGIN_HEIGHT);
    SDL_RenderDrawLine(renderer, MARGIN_WIDTH, MARGIN_HEIGHT, MARGIN_WIDTH, HEIGHT - MARGIN_HEIGHT);
    SDL_RenderDrawLine(renderer, WIDTH - MARGIN_WIDTH, MARGIN_HEIGHT, WIDTH - MARGIN_WIDTH, HEIGHT - MARGIN_HEIGHT);
    for (int i = 0; i <= NUM_TICKS; i++) {
        float tick_coords_d[2] = {boundaries.xInterval.min + i*(boundaries.xInterval.max - boundaries.xInterval.min)/NUM_TICKS, (float) (HEIGHT - MARGIN_HEIGHT)};
        getCoords(tick_coords_d, boundaries);
        int tick_coords[2] = {(int) tick_coords_d[0], (int) tick_coords_d[1]};
        // not using tick_coords[1] bc not point on the graph, so it's not the right place
        // drawing ticks

        int tick_size = 10; 
        SDL_RenderDrawLine(renderer, tick_coords[0], HEIGHT - MARGIN_HEIGHT, tick_coords[0], HEIGHT - MARGIN_HEIGHT + tick_size);

        //render text
        float x_val = boundaries.xInterval.min + i * (boundaries.xInterval.max - boundaries.xInterval.min) / NUM_TICKS;
        char label[10];
        snprintf(label, sizeof(label), "%.1f", x_val);

        int text_height = 0;
        int text_width = 0;
        TTF_SizeText(font, label, &text_width, &text_height);

        drawText(renderer, font, label, tick_coords[0] - text_width/2, HEIGHT - MARGIN_HEIGHT + tick_size);
    }

    for (int i = 0; i <= NUM_TICKS; i++) {
        float tick_coords_d[2] = {(float) (WIDTH - MARGIN_WIDTH), boundaries.yInterval.min + i*(boundaries.yInterval.max - boundaries.yInterval.min)/NUM_TICKS};
        getCoords(tick_coords_d, boundaries);
        int tick_coords[2] = {(int) tick_coords_d[0], (int) tick_coords_d[1]};
        // not using tick_coords[0] bc not point on the graph, so it's not the right place
        // drawing ticks

        int tick_size = 10; 
        SDL_RenderDrawLine(renderer, MARGIN_WIDTH - tick_size, tick_coords[1], MARGIN_WIDTH, tick_coords[1]);

        //render text
        float y_val = boundaries.yInterval.min + i * (boundaries.yInterval.max - boundaries.yInterval.min) / NUM_TICKS;
        char label[10];
        snprintf(label, sizeof(label), "%.1f", y_val);

        int text_height = 0;
        int text_width = 0;
        TTF_SizeText(font, label, &text_width, &text_height);

        drawText(renderer, font, label, MARGIN_WIDTH - tick_size - text_width - 3, tick_coords[1] - text_height / 2);
    }
}

void drawGraph(SDL_Renderer* renderer, float* data, int length, graphBoundaries boundaries, TTF_Font* font) {
    
    // graphs the data inputted. max_value is the maximum of the absolute values of data.
    float max_value = maxAbs(boundaries.yInterval.max, boundaries.yInterval.min);
    float point1[2];
    float point2[2];
    float point2_temp[2];
    for (int i = 0; i < length - 1; i++) {
        // map data points to window size (scale to fit vertically)

        if(i == 0){
            point1[0] = boundaries.xInterval.min;
            point1[1] = data[0];
        } else{
            point1[0] = point2_temp[0];
            point1[1] = point2_temp[1];
        }
        point2[0] = boundaries.xInterval.min + (i+1) * (boundaries.xInterval.max - boundaries.xInterval.min) / length;
        point2[1] = data[i+1];
        point2_temp[0] = point2[0];
        point2_temp[1] = point2[1];
        getCoords(point1, boundaries);
        getCoords(point2, boundaries);
        int x1 = (int) point1[0];
        int y1 = (int) point1[1];
        int x2 = (int) point2[0];
        int y2 = (int) point2[1];

        // draw line between two adjacent points
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
    }
    drawBackground(renderer, data, length, boundaries, font);
}