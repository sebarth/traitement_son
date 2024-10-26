#include "graphs.h"
#include "fft.h"
#include "signal_generation.h"
#include "rendering.h"
#include "audio_capture.h"

SDL_Window* window1;
SDL_Window* window2;

SDL_Renderer* renderer1;
SDL_Renderer* renderer2;

TTF_Font* font;

fftwf_plan fft_plan;

float* orderedData;
pthread_mutex_t globalDataLock;

int main() {
    PaStream *stream;
    PaError err;
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

    fftwf_complex* freq_domain = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * SAMPLE_COUNT);
    float* time_domain = (float*) fftwf_malloc(sizeof(float) * SAMPLE_COUNT);

    fft_init(SAMPLE_COUNT, data.samples, fft_data, "fftw_wisdom.txt", &fft_plan);
    signal_init(freq_domain, time_domain, 1500.0f);

    int quit1 = 0;
    int quit2 = 0;
    SDL_Event event;

    pthread_mutex_init(&globalDataLock, NULL);

    loopArgs loop_args = {&boundaries1, &boundaries2, &data, orderedData, fft_data, spectrum, &t, &quit1, &quit2, &globalDataLock};
    //updateArgs update_args = {&data, orderedData, &t, fft_data, spectrum, &quit1, &quit2, &globalDataLock, freq_domain, time_domain, fft_plan};

    //pthread_t updateThread;

    //pthread_create(&updateThread, NULL, updateDataPtr, &update_args);
    
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

    //pthread_join(updateThread, NULL);

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
error:
    Pa_Terminate();
    fprintf(stderr, "Ugh, there's an error : %s\n", Pa_GetErrorText(err));
    return -1;
}
