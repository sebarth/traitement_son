#include "rendering.h"

void init(graphBoundaries* boundaries1, graphBoundaries* boundaries2){
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    window1 = SDL_CreateWindow("Amplitude Graph", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    window2 = SDL_CreateWindow("Spectrum Graph", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    
    renderer1 = SDL_CreateRenderer(window1, -1, SDL_RENDERER_PRESENTVSYNC);
    renderer2 = SDL_CreateRenderer(window2, -1, SDL_RENDERER_PRESENTVSYNC);

    font = TTF_OpenFont("fonts/Roboto-Light.ttf", 16);

    
    //1st window
    //drawing axes
    SDL_SetRenderDrawColor(renderer1, 255, 255, 255, 255); // Set to white background
    SDL_RenderClear(renderer1);
    SDL_SetRenderTarget(renderer1, NULL);
    boundaries1->xInterval.min = -(float)SAMPLE_COUNT / SAMPLE_RATE;
    boundaries1->xInterval.max = 0.0f;
    boundaries1->yInterval.min = -1.0f;
    boundaries1->yInterval.max = 1.0f;

    //2nd window
    SDL_SetRenderDrawColor(renderer2, 255, 255, 255, 255); // Set to white background
    SDL_RenderClear(renderer2);
    SDL_SetRenderTarget(renderer2, NULL);
    boundaries2->xInterval.min = 0.0f;
    //using f_k = k.F_s/N
    //max val of k : SAMPLE_COUNT/2 --> SAMPLE_COUNT / 2 * SAMPLE_RATE / SAMPLE_COUNT = SAMPLE_RATE / 2
    boundaries2->xInterval.max = SAMPLE_RATE / 2.0f;
    boundaries2->yInterval.min = 0.0f;
    boundaries2->yInterval.max = 1.0f;
}

void loop(loopArgs args){
    if (window1){
        //update all boundaries to fit the graph
        /*(args.boundaries1)->yInterval.max = dataMax(args.orderedData, SAMPLE_COUNT);
        (args.boundaries1)->yInterval.min = dataMin(args.orderedData, SAMPLE_COUNT);*/
        

        SDL_SetRenderDrawColor(renderer1, 255, 255, 255, 255);
        SDL_RenderClear(renderer1);

        SDL_SetRenderDrawColor(renderer1, 255, 0, 0, 255);
        drawGraph(renderer1, args.orderedData, SAMPLE_COUNT, *(args.boundaries1), font);

        SDL_RenderPresent(renderer1);
    }
    if (window2){
        //update yInterval's max value to fit (min is always 0 so don't update min)
        (args.boundaries2)->yInterval.max = dataMax(args.spectrum, SAMPLE_COUNT / 2 + 1);

        SDL_SetRenderDrawColor(renderer2, 255, 255, 255, 255);
        SDL_RenderClear(renderer2);

        SDL_SetRenderDrawColor(renderer2, 0, 0, 255, 255);
        drawGraph(renderer2, args.spectrum, SAMPLE_COUNT / 2 + 1, *(args.boundaries2), font);

        SDL_RenderPresent(renderer2);
    }
}