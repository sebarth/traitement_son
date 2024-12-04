#include "graphs.h"

void drawText(SDL_Renderer* renderer, TTF_Font* font, const char* text, int x, int y, SDL_Color color) {
    if (font == NULL) {
        fprintf(stderr, "Font not loaded correctly.\n");
        return;  // Or handle the error more gracefully
    }
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text, color);
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
    coords[0] = roundf(a_x * coords[0] + b_x);

    // do the same for y
    float a_y = (float) (graph_height) / (boundaries.yInterval.max - boundaries.yInterval.min);
    float b_y = (float) (MARGIN_HEIGHT + graph_height + graph_margin_height) -  boundaries.yInterval.max * a_y;
    coords[1] = (float) HEIGHT - roundf(a_y * coords[1] + b_y);

    return;
}

void drawTicks(SDL_Renderer* renderer, graphBoundaries boundaries, TTF_Font* font){
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
        snprintf(label, sizeof(label), "%.2f", x_val);

        int text_height = 0;
        int text_width = 0;
        int error = TTF_SizeUTF8(font, label, &text_width, &text_height);

        drawText(renderer, font, label, tick_coords[0] - text_width/2, HEIGHT - MARGIN_HEIGHT + tick_size, (SDL_Color){0, 0, 0, 255});
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
        snprintf(label, sizeof(label), "%.3f", y_val);

        int text_height = 0;
        int text_width = 0;
        TTF_SizeText(font, label, &text_width, &text_height);

        drawText(renderer, font, label, MARGIN_WIDTH - tick_size - text_width - 3, tick_coords[1] - text_height / 2, (SDL_Color){0, 0, 0, 255});
    }
}

void drawBackground(SDL_Renderer* renderer, float* data, int length, graphBoundaries boundaries, TTF_Font* font){
    //draw axes
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawLine(renderer, MARGIN_WIDTH, MARGIN_HEIGHT, WIDTH - MARGIN_WIDTH, MARGIN_HEIGHT);
    SDL_RenderDrawLine(renderer, MARGIN_WIDTH, HEIGHT - MARGIN_HEIGHT, WIDTH - MARGIN_WIDTH, HEIGHT - MARGIN_HEIGHT);
    SDL_RenderDrawLine(renderer, MARGIN_WIDTH, MARGIN_HEIGHT, MARGIN_WIDTH, HEIGHT - MARGIN_HEIGHT);
    SDL_RenderDrawLine(renderer, WIDTH - MARGIN_WIDTH, MARGIN_HEIGHT, WIDTH - MARGIN_WIDTH, HEIGHT - MARGIN_HEIGHT);
    drawTicks(renderer, boundaries, font);
}

void drawLegend(SDL_Renderer* renderer, TTF_Font* font, char* legendx, char* legendy){
    drawText(renderer, font, legendx, WIDTH - MARGIN_WIDTH + 20, HEIGHT - MARGIN_HEIGHT, (SDL_Color){0, 0, 0, 255});
    drawText(renderer, font, legendy, MARGIN_WIDTH - 50, MARGIN_HEIGHT - 20, (SDL_Color){0, 0, 0, 255});
}

void drawCurve(SDL_Renderer* renderer, float* data, int length, graphBoundaries boundaries){
    // graphs the inputted data.
    // we draw a line between each pair of adjacent points
    float point1[2]; // store the current first point
    float point2[2]; // store the current second point
    float point2_temp[2]; // store the previous point
    for (int i = 0; i < length - 1; i++) {
        // map data points to window size (scale to fit vertically)
        if(i == 0){
            point1[0] = boundaries.xInterval.min; // start at the left edge
            point1[1] = data[0]; // start at the first data point
        } else{
            point1[0] = point2_temp[0]; // start at the previous point
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
}

void drawGraph(SDL_Renderer* renderer, float* data, int length, graphBoundaries boundaries, TTF_Font* font, char* legendx, char* legendy){
    drawBackground(renderer, data, length, boundaries, font);
    drawLegend(renderer, font, legendx, legendy);
    drawCurve(renderer, data, length, boundaries);
}