#include <math.h>
#include <SDL2/SDL.h> //graphs
#include <SDL2/SDL_ttf.h> //texts
#include <stdlib.h>
#include <fftw3.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "graphs.h"

const int SAMPLE_COUNT = 1024;
const int RATE = 10000;
const int FRAMES_PER_BUFFER = 1;

SDL_Window* window1;
SDL_Window* window2;

SDL_Renderer* renderer1;
SDL_Renderer* renderer2;

TTF_Font* font;
fftwf_complex* freq_domain;
float* time_domain;

fftwf_plan fft_plan;

typedef struct {
    float *samples;   //the data
    int maxFrameIndex; //total number of indexes
    int currentIndex;  //next index to update
} AudioData;
typedef struct{
    graphBoundaries* boundaries1;
    graphBoundaries* boundaries2;
    AudioData* data;
    float* orderedData;
    fftwf_complex* fft_data;
    float* spectrum;
    float* t;
    int* quit1;
    int* quit2;
    pthread_mutex_t* globalDataLock;
} loopArgs;
typedef struct{
    AudioData* data;
    float* orderedData;
    float* t;
    fftwf_complex* fft_data;
    float* spectrum;
    int* quit1;
    int* quit2;
    pthread_mutex_t* globalDataLock;
} updateArgs;

void init(){
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    window1 = SDL_CreateWindow("Amplitude Graph", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    window2 = SDL_CreateWindow("Spectrum Graph", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    
    renderer1 = SDL_CreateRenderer(window1, -1, SDL_RENDERER_PRESENTVSYNC);
    renderer2 = SDL_CreateRenderer(window2, -1, SDL_RENDERER_PRESENTVSYNC);

    font = TTF_OpenFont("fonts/Roboto-Light.ttf", 16);
}

void signal_init() {
    float freq = 1500.0;
    freq_domain = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * SAMPLE_COUNT);
    time_domain = (float*) fftwf_malloc(sizeof(float) * SAMPLE_COUNT);
    
    fftwf_plan plan = fftwf_plan_dft_c2r_1d(SAMPLE_COUNT, freq_domain, time_domain, FFTW_ESTIMATE);
    
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        freq_domain[i][0] = 0.0;
        freq_domain[i][1] = 0.0;
    }

    // Find the index of the 1500 Hz bin
    int bin = (int)((freq * SAMPLE_COUNT) / RATE);
    freq_domain[bin][0] = SAMPLE_COUNT / 2.0;  // Amplitude (real part)
    freq_domain[bin][1] = 0.0;      // No imaginary component for a real sine wave

    // Compute the inverse FFT
    fftwf_execute(plan);

    // Normalize the output of the IFFT
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        time_domain[i] /= SAMPLE_COUNT;  // FFTW does not normalize automatically
    }
}

float signal(float t){
    int index = (int)((t * RATE))%SAMPLE_COUNT;
    return time_domain[index];
}

void orderData(AudioData data, float* orderedData){
    if (data.currentIndex == data.maxFrameIndex - 1) {
        // if current index is max index, copy the whole table bc already ordered
        memcpy(orderedData, data.samples, data.maxFrameIndex * sizeof(float));
    } else{
        // else, copy first the last values (least recent)
        memcpy(orderedData, data.samples + data.currentIndex, (data.maxFrameIndex - data.currentIndex) * sizeof(float));
        // then copy the first values until currentIndex (because currentIndex is the most recent)
        memcpy(orderedData + data.maxFrameIndex - data.currentIndex, data.samples, (data.currentIndex) * sizeof(float));
    }
}
void updateFFTData(updateArgs args){
    fftwf_execute(fft_plan);

    //update spectrum
    for (int i = 0; i < SAMPLE_COUNT; i++){
        args.spectrum[i] = sqrt(args.fft_data[i][0] * args.fft_data[i][0] + args.fft_data[i][1] * args.fft_data[i][1]);
    }
}

void updateData(updateArgs args){
    //update data variable
    for (unsigned long i = 0; i < FRAMES_PER_BUFFER; i++) {
            // Convertir et stocker les échantillons dans le tampon circulaire
            args.data->samples[args.data->currentIndex] = signal(*(args.t));
            args.data->currentIndex = (args.data->currentIndex + 1) % args.data->maxFrameIndex;
            *(args.t) += 1.0 / RATE;
            if (*(args.t) >= 1.0) *(args.t) -= 1.0;
    }
    //update t (only for now generating data)
    /* *(args.t) += 1;
    *(args.t) %= 750; */
}

void loop(loopArgs args){
    // !!!!!! /!\ LOOPING /!\ !!!!!!
    if (window1){
        //update all boundaries to fit the graph
        (args.boundaries1)->yInterval.max = dataMax(args.orderedData, SAMPLE_COUNT);
        (args.boundaries1)->yInterval.min = dataMin(args.orderedData, SAMPLE_COUNT);

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

void* updateDataPtr(void* update_args){
    updateArgs args = *(updateArgs*)update_args;

    while (!*(args.quit1) || !*(args.quit2)) {
        pthread_mutex_lock(args.globalDataLock);
        updateData(args);
        orderData(*(args.data), args.orderedData);
        updateFFTData(args);
        pthread_mutex_unlock(args.globalDataLock);

        usleep(1000000 / (RATE * FRAMES_PER_BUFFER));
    }
    return NULL;
}

int main() {
    init();

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

    AudioData data;
    data.maxFrameIndex = SAMPLE_COUNT; // 5 secs
    data.samples = (float*)malloc(sizeof(float) * data.maxFrameIndex);
    data.currentIndex = 0;
    float *orderedData = (float*)malloc(sizeof(float) * data.maxFrameIndex);

    float t = 0;

    fftwf_complex *fft_data = (fftwf_complex*)fftwf_malloc(SAMPLE_COUNT * sizeof(fftwf_complex));
    float *spectrum = malloc(SAMPLE_COUNT * sizeof(float));

    fftwf_import_wisdom_from_filename("fftw_wisdom.txt");
    fft_plan = fftwf_plan_dft_r2c_1d(SAMPLE_COUNT, data.samples, fft_data, FFTW_ESTIMATE);


    int quit1 = 0; // so that you stop updating the 1st window
    int quit2 = 0; // same for 2nd
    SDL_Event event;

    signal_init();
    pthread_mutex_t globalDataLock;

    pthread_mutex_init(&globalDataLock, NULL);
    

    loopArgs loop_args = {&boundaries1, &boundaries2, &data, orderedData, fft_data, spectrum, &t, &quit1, &quit2, &globalDataLock};
    updateArgs update_args = {&data, orderedData, &t, fft_data, spectrum, &quit1, &quit2, &globalDataLock};

    pthread_t updateThread;

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
                    SDL_DestroyWindow(window1);
                    window1 = NULL;
                    quit1 = 1;
                }
                if (event.window.windowID == SDL_GetWindowID(window2)) {
                    SDL_DestroyWindow(window2);
                    window2 = NULL;
                    quit2 = 1;
                }
            }
        }
        pthread_mutex_lock(&globalDataLock);
        loop(loop_args);
        pthread_mutex_unlock(&globalDataLock);

        usleep(1000000 / 60); // Update at ~60 frames per second
    } 


    pthread_join(updateThread, NULL);

    pthread_mutex_destroy(&globalDataLock);

    free(data.samples);
    free(spectrum);
    fftwf_free(freq_domain);
    fftwf_free(fft_data);
    if (renderer1) SDL_DestroyRenderer(renderer1);
    if (renderer2) SDL_DestroyRenderer(renderer2);
    if (window1) SDL_DestroyWindow(window1);
    if (window2) SDL_DestroyWindow(window2);
    SDL_Quit();
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

    |^^^^^^^^^^^^^^|
    |.____.||.____.|
   <|\.___.||.___./|>
    | \.__.||.__./ |
   <|  \._.||._./  |>
    |   \..||../   |
   <|    \.||./    |>
    |     \||/     |
   <|=====|°°|=====|>
    |-----/..\-----|
   <|====/.__.\====|>
    |---/.____.\---|
   <|==/.______.\==|>
    |-/.________.\-|
   <|/.__________.\|>
    |.____________.|


THE FINAL FIGHT - SDL_RENDERING vs MULTI_THREADING


HERE IS YOUR ARMY :
._______________________________________________________.
|     ___     |     ___     |     ___     |     ___     |  
|   .|ç-ç|.   |   .|è-è|.   |   .|#-#|.   |   .|@-@|.   |  
|    |___|    |    |___|    |    |___|    |    |___|    |  
|     | |     |     | |     |     | |     |     | |     |  
|   Captain   |   Whisper   |    Gizmo    |    Quirk    |  
| ______________________________________________________|
|     ___     |     ___     |     ___     |     ___     |  
|   .|*_*|.   |   .|~_~|.   |   .|°-°|.   |   .|=_=|.   |  
|    |___|    |    |___|    |    |___|    |    |___|    |  
|     | |     |     | |     |     | |     |     | |     |  
|  Starlight  |   Snoozer   |   Oddball   |   Glitch    |  
| ______________________________________________________|
|     ___     |     ___     |     ___     |     ___     |  
|   .|µ-µ|.   |   .|;-;|.   |   .|»-»|.   |   .|UwU|.   |  
|    |___|    |    |___|    |    |___|    |    |___|    |  
|     | |     |     | |     |     | |     |     | |     |  
|   Mysterio  | Melancholy  |   Voyager   |    Kawaii   |  
|_______________________________________________________|



HERE'S THE BOSS :

            \   .________________.   /
             \  |                |  /
              \ |   (°)    (°)   | /
               \|   (°)    (°)   |/
            \  /|   (°)    (°)   |\  /
             \/ |   (°)    (°)   | \/
             /\ |                | /\
            /  \|                |/  \
               /|                |\
              / .________________. \
             /                      \
            /                        \
                THE FALSE SPYDER
              (newbie python spider) 
            
            \   .________________
             \  |                   
              \ |   (°)         
               \|   (°)    (°)    
            \  /|   (°)    (°)      
             \/ |   (°)    (°)     
             /\ |          (°)        
            /  \|                   
               /|                 
              / .________________
             /                      
            /                        
                 THE 1 CPYDER 
         (not spyder cuz it's in C lol)

TIP1 : NOTE THAT SDL RENDERING ONLY WORKS ON THE MAIN THREAD ._.
TIP2 : THAT's ALL...........................
                                  






*/
