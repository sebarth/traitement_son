#include <SDL2/SDL.h> //graphs
#include <SDL2/SDL_ttf.h> //texts
#include <math.h>
#include <stdlib.h>
#include <complex.h>
#include <pthread.h>
#include <unistd.h>
#include "graphs.h"
#include "fft.h"

SDL_Window* window1;
SDL_Window* window2;

SDL_Renderer* renderer1;
SDL_Renderer* renderer2;

TTF_Font* font;

pthread_mutex_t rendererLock;

typedef struct{
    graphBoundaries* boundaries1;
    graphBoundaries* boundaries2;
    double* data;
    double complex* fft_data;
    double* spectrum;
    int* t;
    int* quit1;
    int* quit2;
    pthread_mutex_t* globalDataLock;
} loopArgs;
typedef struct{
    double* data; 
    int* t;
    double complex* fft_data;
    double* spectrum;
    int* quit1;
    int* quit2;
    pthread_mutex_t* globalDataLock;
} updateArgs;

void init_sdl(){
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    window1 = SDL_CreateWindow("Amplitude Graph", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    window2 = SDL_CreateWindow("Spectrum Graph", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    
    renderer1 = SDL_CreateRenderer(window1, -1, SDL_RENDERER_PRESENTVSYNC);
    renderer2 = SDL_CreateRenderer(window2, -1, SDL_RENDERER_PRESENTVSYNC);

    font = TTF_OpenFont("fonts/Roboto-Light.ttf", 16);
}

double signal(double t){
    //x(t), the signal
    return sin(2 * M_PI * t / (SAMPLE_COUNT / 2));
}

void updateData(updateArgs args){
    double temp = args.data[0];
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        args.data[i] = signal(i + *(args.t));
        args.fft_data[i] = args.data[i] + 0*I;
    }
    args.data[SAMPLE_COUNT - 1] = signal(SAMPLE_COUNT - 1 + *(args.t));
    fft(args.fft_data, SAMPLE_COUNT, LOG2_SAMPLE_COUNT);
    for (int i = 0; i < SAMPLE_COUNT; i++){
        args.spectrum[i] = cabs(args.fft_data[i]);
    }
    *(args.t) += 1;
    *(args.t) %= 8 * SAMPLE_COUNT;
}

void loop(loopArgs args){
    // !!!!!! /!\ LOOPING /!\ !!!!!!
    if (!*(args.quit1)){
        //update all boundaries to fit
        pthread_mutex_lock(&rendererLock);
        (args.boundaries1)->yInterval.max = dataMax(args.data, SAMPLE_COUNT);
        (args.boundaries1)->yInterval.min = dataMin(args.data, SAMPLE_COUNT); 

        SDL_SetRenderDrawColor(renderer1, 225, 225, 225, 255);
        SDL_RenderClear(renderer1);

        SDL_SetRenderDrawColor(renderer1, 255, 0, 0, 255);
        drawGraph(renderer1, args.data, SAMPLE_COUNT, *(args.boundaries1), font);

        SDL_RenderPresent(renderer1);
        pthread_mutex_unlock(&rendererLock);
    }
    if (!*(args.quit2)){
        pthread_mutex_lock(&rendererLock);
        //update yInterval's max value to fit (min is always 0 so don't update min)
        (args.boundaries2)->yInterval.max = dataMax(args.spectrum, SAMPLE_COUNT);


        SDL_SetRenderDrawColor(renderer2, 225, 225, 225, 255);
        SDL_RenderClear(renderer2);

        SDL_SetRenderDrawColor(renderer2, 255, 0, 0, 255);
        drawGraph(renderer2, args.spectrum, SAMPLE_COUNT / 2 + 1, *(args.boundaries2), font);

        SDL_RenderPresent(renderer2);
        pthread_mutex_unlock(&rendererLock);
    }
}

void* updateDataPtr(void* update_args){
    updateArgs args = *(updateArgs*)update_args;

    while (!*(args.quit1) || !*(args.quit2)) {
        pthread_mutex_lock(args.globalDataLock);
        updateData(args);
        pthread_mutex_unlock(args.globalDataLock);

        usleep(25000);
    }
    printf("exiting update\n");
    return NULL;
}

void* loopPtr(void* loop_args){
    loopArgs args = *(loopArgs*)loop_args;
    while (!*(args.quit1) || !*(args.quit2)){
        pthread_mutex_lock(args.globalDataLock);
        loop(args);
        pthread_mutex_unlock(args.globalDataLock);
        usleep(1000000 / 60); // Update at ~60 frames per second
    }
    printf("exiting loop\n");
    return NULL;
}

int main() {
    init_sdl();

    //1st window
    //drawing axes
    SDL_SetRenderDrawColor(renderer1, 255, 255, 255, 255); // Set to white background
    SDL_RenderClear(renderer1);
    SDL_SetRenderTarget(renderer1, NULL);
    Interval xInterval1;
    Interval yInterval1;
    xInterval1.min = -5.0;
    xInterval1.max = 0.0;
    yInterval1.min = -1.0;
    yInterval1.max = 1.0;
    graphBoundaries boundaries1 = {xInterval1, yInterval1};

    //2nd window
    SDL_SetRenderDrawColor(renderer2, 255, 255, 255, 255); // Set to white background
    SDL_RenderClear(renderer2);
    SDL_SetRenderTarget(renderer2, NULL);
    Interval xInterval2;
    xInterval2.min = 0.0;
    //using f_k = k.F_s/N
    //max val of k : SAMPLE_COUNT/2 --> SAMPLE_COUNT / 2 * RATE / SAMPLE_COUNT = RATE / 2
    xInterval2.max = RATE / 2.0;
    Interval yInterval2;
    yInterval2.min = 0.0;
    graphBoundaries boundaries2 = {xInterval2, yInterval2};

    double *data = malloc(SAMPLE_COUNT * sizeof(double));
    if (data == NULL) printf("data");
    int t = 0;

    double complex *fft_data = malloc(SAMPLE_COUNT * sizeof(double complex));
    if (fft_data == NULL) printf("fft_data");
    double *spectrum = malloc(SAMPLE_COUNT * sizeof(double));
    if (spectrum == NULL) printf("spectrum");

    int quit1 = 0; // so that you stop updating the 1st window
    int quit2 = 0; // same for 2nd
    SDL_Event event;

    pthread_mutex_t globalDataLock;

    pthread_mutex_init(&globalDataLock, NULL);
    pthread_mutex_init(&rendererLock, NULL);
    

    loopArgs loop_args = {&boundaries1, &boundaries2, data, fft_data, spectrum, &t, &quit1, &quit2, &globalDataLock};
    updateArgs update_args = {data, &t, fft_data, spectrum, &quit1, &quit2, &globalDataLock};

    pthread_t loopThread, updateThread;

    pthread_create(&loopThread, NULL, loopPtr, &loop_args);
    pthread_create(&updateThread, NULL, updateDataPtr, &update_args);


    while (!quit1 || !quit2) {
        // Event handling
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit1 = 1;
                quit2 = 1;
            }
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE) {
                if (event.window.windowID == SDL_GetWindowID(window1)) {
                    SDL_DestroyRenderer(renderer1);
                    renderer1 = NULL;
                    SDL_DestroyWindow(window1);
                    window1 = NULL;
                    quit1 = 1;
                }
                if (event.window.windowID == SDL_GetWindowID(window2)) {
                    SDL_DestroyRenderer(renderer2);
                    renderer2 = NULL;
                    SDL_DestroyWindow(window2);
                    window2 = NULL;
                    quit2 = 1;
                }
            }
        }
        SDL_Delay(10);  // Small delay to reduce CPU usage
    } 

    printf("I'm outta there");

    pthread_join(loopThread, NULL);
    pthread_join(updateThread, NULL);
    printf("Yup, made it to line 207\n");

    pthread_mutex_destroy(&globalDataLock);
    pthread_mutex_destroy(&rendererLock);
    printf("Yup, made it to line 210\n");

    free(data);
    free(fft_data);
    free(spectrum);
    if (renderer1) SDL_DestroyRenderer(renderer1);
    if (renderer2) SDL_DestroyRenderer(renderer2);
    if (window1) SDL_DestroyWindow(window1);
    if (window2) SDL_DestroyWindow(window2);
    printf("I made it 'til the end...\n");
    SDL_Quit();
    printf("did I ?\n");
    return 0;
}

/*

         _____
         /°_°\
        /|   |\
      _/ |   | \_
         \___/
         |   |
        _|   |_
       Here's Bob. 
He just said he wanted to
     read this code.
         _____
    .____/>_<\____.
         |   |
         |   |
         \___/
         |   |
        _|   |_
     This is also Bob. 
  He just read this code.

*/
