#include "graphs.h"
#include "fft.h"
#include "signal_generation.h"
#include "rendering.h"

SDL_Window* window1;
SDL_Window* window2;

SDL_Renderer* renderer1;
SDL_Renderer* renderer2;

TTF_Font* font;

fftwf_plan fft_plan;

int main() {
    graphBoundaries boundaries1;
    graphBoundaries boundaries2;
    init(&boundaries1, &boundaries2);

    AudioData data;
    data.maxFrameIndex = SAMPLE_COUNT;
    data.currentIndex = 0;
    float *orderedData;

    data.samples = (float*)malloc(sizeof(float) * data.maxFrameIndex);
    orderedData = (float*)malloc(sizeof(float) * data.maxFrameIndex);

    float t = 0;

    fftwf_complex *fft_data = (fftwf_complex*)fftwf_malloc(SAMPLE_COUNT * sizeof(fftwf_complex));
    float *spectrum = malloc(SAMPLE_COUNT * sizeof(float));

    fftwf_complex* freq_domain = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * SAMPLE_COUNT);
    float* time_domain = (float*) fftwf_malloc(sizeof(float) * SAMPLE_COUNT);

    fft_init(SAMPLE_COUNT, data.samples, fft_data, "fftw_wisdom.txt", &fft_plan);
    signal_init(freq_domain, time_domain, 1500.0f);

    int quit1 = 0;
    int quit2 = 0;
    SDL_Event event;

    pthread_mutex_t globalDataLock;

    pthread_mutex_init(&globalDataLock, NULL);

    loopArgs loop_args = {&boundaries1, &boundaries2, &data, orderedData, fft_data, spectrum, &t, &quit1, &quit2, &globalDataLock};
    updateArgs update_args = {&data, orderedData, &t, fft_data, spectrum, &quit1, &quit2, &globalDataLock, freq_domain, time_domain, fft_plan};

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
    
    fftwf_destroy_plan(fft_plan);

    free(data.samples);
    free(spectrum);
    free(orderedData);
    free(time_domain);
    fftwf_free(fft_data);
    fftwf_free(freq_domain);
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
TIP2 : THAT's ALL.............................
*/
