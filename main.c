#include "libs.h"
#include "constants.h"
#include "cross_platform.h"
#include "graphs.h"
#include "fft.h"
#include "rendering.h"
#include "audio_capture.h"

SDL_Window* main_window;

SDL_Renderer* main_renderer;

TTF_Font* font;
TTF_Font* buttonFont;
TTF_Font* legendFont;

fftwf_plan fft_plan;

float* orderedData;
pthread_mutex_t globalDataLock;

int main_function() {
    PaStream *stream;
    PaError err;
    graphBoundaries boundaries1;
    graphBoundaries boundaries2;
    Button changeWindowButton;
    init(&boundaries1, &boundaries2, &changeWindowButton);

    AudioData data;
    data.maxFrameIndex = SAMPLE_COUNT;
    data.currentIndex = 0;
    float *orderedData;

    data.samples = (float*)malloc(sizeof(float) * data.maxFrameIndex);
    if (data.samples == NULL){
        fprintf(stderr, "Malloc failed for data.samples\n");
        return -1;
    }
    orderedData = (float*)malloc(sizeof(float) * data.maxFrameIndex);
    if (orderedData == NULL){
        fprintf(stderr, "Malloc failed for orderedData\n");
        free(data.samples); // Free previously allocated memory
        return -1;
    }

    float t = 0;

    fftwf_complex *fft_data = (fftwf_complex*)fftwf_malloc(SAMPLE_COUNT * sizeof(fftwf_complex));
    if (fft_data == NULL){
        fprintf(stderr, "Malloc failed for fft_data\n");
        free(data.samples); // Free previously allocated memory
        free(orderedData); // Free previously allocated memory
        return -1;
    }

    float *spectrum = malloc(SAMPLE_COUNT * sizeof(float));
    if (spectrum == NULL){
        fprintf(stderr, "Malloc failed for spectrum\n");
        free(data.samples); // Free previously allocated memory
        free(orderedData); // Free previously allocated memory
        fftwf_free(fft_data); // Free previously allocated memory
        return -1;
    }

    fftwf_complex* freq_domain = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * SAMPLE_COUNT);
    if (freq_domain == NULL){
        fprintf(stderr, "Malloc failed for freq_domain\n");
        free(data.samples); // Free previously allocated memory
        free(orderedData); // Free previously allocated memory
        fftwf_free(fft_data); // Free previously allocated memory
        free(spectrum); // Free previously allocated memory
        return -1;
    }
    
    float* time_domain = (float*) fftwf_malloc(sizeof(float) * SAMPLE_COUNT);
    if (time_domain == NULL){
        fprintf(stderr, "Malloc failed for time_domain\n");
        free(data.samples); // Free previously allocated memory
        free(orderedData); // Free previously allocated memory
        fftwf_free(fft_data); // Free previously allocated memory
        free(spectrum); // Free previously allocated memory
        fftwf_free(freq_domain); // Free previously allocated memory
        return -1;
    }
    printf("Everything correctly allocated\n");

    fft_init(SAMPLE_COUNT, data.samples, fft_data, "fftw_wisdom.txt", &fft_plan);

    int quit = 0;
    int currentWindow = 1;
    SDL_Event event;

    pthread_mutex_init(&globalDataLock, NULL);

    loopArgs loop_args = {&boundaries1, &boundaries2, &data, orderedData, fft_data, spectrum, &t, &quit, &globalDataLock, currentWindow, &changeWindowButton, {255, 0, 0, 255}, {0, 0, 255, 255}};
    
    err = Pa_Initialize();
    if (err != paNoError) goto error;

    err = Pa_OpenDefaultStream(&stream,
                               1,          // mono input
                               0,          // no output
                               paFloat32,  // format 32 bits float
                               SAMPLE_RATE,
                               FRAMES_PER_BUFFER,
                               customAudioCallback,
                               &data);
    if (err != paNoError) goto error;

    err = Pa_StartStream(stream);
    if (err != paNoError) goto error;

    while (!quit) {
        // Event handling
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = 1;
            }
            if (event.type == SDL_MOUSEMOTION) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                changeWindowButton.isHovered = isMouseOverButton(&changeWindowButton, x, y);
            } 
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT && changeWindowButton.isHovered) {
                changeWindowButton.onClick((void*)&loop_args);
            } 
        }
        copySamplesInOrder(&data, orderedData);
        updateFFTData(fft_data, spectrum, SAMPLE_COUNT, fft_plan);
        pthread_mutex_lock(&globalDataLock);
        loop(loop_args);
        pthread_mutex_unlock(&globalDataLock);

        usleep((int)1e6 / 60); // Update at ~60 frames per second
    }

    err = Pa_StopStream(stream);
    if (err != paNoError) goto error;

    err = Pa_CloseStream(stream);
    if (err != paNoError) goto error;

    Pa_Terminate();

    pthread_mutex_destroy(&globalDataLock);
    
    fftwf_destroy_plan(fft_plan);

    free(data.samples);
    free(spectrum);
    free(orderedData);
    free(time_domain);
    fftwf_free(fft_data);
    fftwf_free(freq_domain);
    SDL_DestroyRenderer(main_renderer);
    SDL_DestroyWindow(main_window);
    SDL_Quit();
    return 0;
error:
    if (data.samples) free(data.samples);
    if (orderedData) free(orderedData);
    if (fft_data) fftwf_free(fft_data);
    if (spectrum) free(spectrum);
    if (freq_domain) fftwf_free(freq_domain);
    if (time_domain) free(time_domain);
    SDL_Quit();
    Pa_Terminate();
    fprintf(stderr, "Ugh, there's an error : %s\n", Pa_GetErrorText(err));
    return -1;
}
// if windows, main_function is WinMain, else main_function is main 
#ifdef _WIN32
#define ENTRY_POINT WinMain
#else
#define ENTRY_POINT main
#endif

int ENTRY_POINT (){
    return main_function();
}