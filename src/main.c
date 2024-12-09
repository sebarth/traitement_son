#include "libs.h"
#include "constants.h"
#include "cross_platform.h"
#include "graphs.h"
#include "math_tools.h"
#include "rendering.h"
#include "audio_capture.h"
#include "raw_knn_data.h"

SDL_Window* main_window;

SDL_Renderer* main_renderer;

fftwf_plan fft_plan;

pthread_mutex_t globalDataLock;

int main_function() {
    PaStream *stream;
    PaError err;

    graphBoundaries boundaries1;
    graphBoundaries boundaries2;
    graphBoundaries boundaries3;
    Button changeWindowButton;

    ViewType currentView = VIEW_INPUT;

    AudioData data; // raw data
    data.maxFrameIndex = SAMPLE_COUNT;
    data.currentIndex = 0;
    float* orderedData;
    float* windowed_data;
    float* precomputed_hamming;
    float* smoothed_spectrum;
    float* autocorr;
    int* spectrum_peaks;
    int* autocorr_peaks;
    int max_peaks_spectrum = 5;
    int max_peaks_autocorr = 20;

    float energy = 0.0f;
    float energy_threshold = 0.1f;
    char* predicted_label;
    int vowel_prediction = 0;

    data.samples = (float*)malloc(sizeof(float) * data.maxFrameIndex);
    if (data.samples == NULL){
        fprintf(stderr, "Malloc failed for data.samples\n");
        return -1;
    } else {
        memset(data.samples, 0, sizeof(float) * data.maxFrameIndex);
    }
    orderedData = (float*)malloc(sizeof(float) * data.maxFrameIndex);
    if (orderedData == NULL){
        fprintf(stderr, "Malloc failed for orderedData\n");
        free(data.samples); // Free previously allocated memory
        return -1;
    } else {
        memset(orderedData, 0, sizeof(float) * data.maxFrameIndex);
    }
    windowed_data = (float*)malloc(sizeof(float) * data.maxFrameIndex);
    if (windowed_data == NULL){
        fprintf(stderr, "Malloc failed for windowed_data\n");
        free(data.samples); // Free previously allocated memory
        free(orderedData); // Free previously allocated memory
        return -1;
    } else {
        memset(windowed_data, 0, sizeof(float) * data.maxFrameIndex);
    }
    precomputed_hamming = (float*)malloc(sizeof(float) * data.maxFrameIndex);
    if (precomputed_hamming == NULL){
        fprintf(stderr, "Malloc failed for precomputed_hamming\n");
        free(data.samples); // Free previously allocated memory
        free(orderedData); // Free previously allocated memory
        free(windowed_data); // Free previously allocated memory
        return -1;
    } else {
        memset(precomputed_hamming, 0, sizeof(float) * data.maxFrameIndex);
    }
    smoothed_spectrum = (float*)malloc(sizeof(float) * (data.maxFrameIndex / 2 + 1));
    if (smoothed_spectrum == NULL){
        fprintf(stderr, "Malloc failed for smoothed_spectrum\n");
        free(data.samples); // Free previously allocated memory
        free(orderedData); // Free previously allocated memory
        free(windowed_data); // Free previously allocated memory
        free(precomputed_hamming); // Free previously allocated memory
        return -1;
    } else {
        memset(smoothed_spectrum, 0, sizeof(float) * (data.maxFrameIndex / 2 + 1));
    }
    autocorr = (float*)malloc(sizeof(float) * data.maxFrameIndex);
    if (autocorr == NULL){
        fprintf(stderr, "Malloc failed for autocorr\n");
        free(data.samples); // Free previously allocated memory
        free(orderedData); // Free previously allocated memory
        free(windowed_data); // Free previously allocated memory
        free(precomputed_hamming); // Free previously allocated memory
        free(smoothed_spectrum); // Free previously allocated memory
        return -1;
    } else {
        memset(autocorr, 0, sizeof(float) * data.maxFrameIndex);
    }
    spectrum_peaks = (int*)malloc(sizeof(int) * max_peaks_spectrum);
    if (spectrum_peaks == NULL){
        fprintf(stderr, "Malloc failed for spectrum_peaks\n");
        free(data.samples); // Free previously allocated memory
        free(orderedData); // Free previously allocated memory
        free(windowed_data); // Free previously allocated memory
        free(precomputed_hamming); // Free previously allocated memory
        free(smoothed_spectrum); // Free previously allocated memory
        free(autocorr); // Free previously allocated memory
        return -1;
    } else {
        memset(spectrum_peaks, 0, sizeof(int) * max_peaks_spectrum);
    }
    autocorr_peaks = (int*)malloc(sizeof(int) * max_peaks_autocorr);
    if (autocorr_peaks == NULL){
        fprintf(stderr, "Malloc failed for autocorr_peaks\n");
        free(data.samples); // Free previously allocated memory
        free(orderedData); // Free previously allocated memory
        free(windowed_data); // Free previously allocated memory
        free(precomputed_hamming); // Free previously allocated memory
        free(smoothed_spectrum); // Free previously allocated memory
        free(autocorr); // Free previously allocated memory
        free(spectrum_peaks); // Free previously allocated memory
        return -1;
    } else {
        memset(autocorr_peaks, 0, sizeof(int) * max_peaks_autocorr);
    }
    predicted_label = (char*)malloc(sizeof(char) * 2);
    if (predicted_label == NULL){
        fprintf(stderr, "Malloc failed for predicted_label\n");
        free(data.samples); // Free previously allocated memory
        free(orderedData); // Free previously allocated memory
        free(windowed_data); // Free previously allocated memory
        free(precomputed_hamming); // Free previously allocated memory
        free(smoothed_spectrum); // Free previously allocated memory
        free(autocorr); // Free previously allocated memory
        free(spectrum_peaks); // Free previously allocated memory
        free(autocorr_peaks); // Free previously allocated memory
        return -1;
    } else {
        memset(predicted_label, 0, sizeof(char) * 2);
    }

    for (int i = 0; i < SAMPLE_COUNT; i++) {
        precomputed_hamming[i] = 0.54f - 0.46f * cosf(2 * M_PI * i / (SAMPLE_COUNT - 1));
    }


    fftwf_complex *fft_data = fftwf_malloc(SAMPLE_COUNT * sizeof(fftwf_complex));
    if (fft_data == NULL){
        fprintf(stderr, "Malloc failed for fft_data\n");
        free(data.samples); // Free previously allocated memory
        free(orderedData); // Free previously allocated memory
        return -1;
    } else {
        memset(fft_data, 0, SAMPLE_COUNT * sizeof(fftwf_complex));
    }

    float *spectrum = malloc(SAMPLE_COUNT * sizeof(float));
    if (spectrum == NULL){
        fprintf(stderr, "Malloc failed for spectrum\n");
        free(data.samples); // Free previously allocated memory
        free(orderedData); // Free previously allocated memory
        fftwf_free(fft_data); // Free previously allocated memory
        return -1;
    } else {
        memset(spectrum, 0, SAMPLE_COUNT * sizeof(float));
    }

    fft_init(SAMPLE_COUNT, windowed_data, fft_data, "fftw_wisdom.txt", &fft_plan);

    int quit = 0;
    int currentWindow = 1;
    SDL_Event event;

    pthread_mutex_init(&globalDataLock, NULL);

    TTF_Font* font;
    TTF_Font* titleFont;
    TTF_Font* legendFont;

    loopArgs loop_args = {
        &boundaries1,
        &boundaries2,
        &boundaries3,
        &data,
        orderedData,
        fft_data,
        smoothed_spectrum,
        autocorr,
        &globalDataLock,
        currentWindow,
        &changeWindowButton,
        {255, 0, 0, 255},
        {0, 0, 255, 255},
        font,
        titleFont,
        legendFont,
        &currentView,
        spectrum_peaks,
        autocorr_peaks,
        max_peaks_spectrum,
        max_peaks_autocorr,
        predicted_label,
        &vowel_prediction
    };

    init(&boundaries1, &boundaries2, &changeWindowButton, &loop_args);
    
    err = Pa_Initialize();
    if (err != paNoError) goto pa_error;

    int numDevices = Pa_GetDeviceCount();
    printf("Number of devices: %d\n", numDevices);
    if (numDevices < 0) {
        fprintf(stderr, "ERROR: Pa_CountDevices returned 0x%x\n", numDevices);
        goto pa_error;
    } else if (numDevices == 0){
        fprintf(stderr, "ERROR: No device is detected.");
        goto pa_error;
    }
    
    PaDeviceIndex defaultInputDevice = Pa_GetDefaultInputDevice();
    if (defaultInputDevice == paNoDevice) {
        fprintf(stderr, "ERROR: Pa_GetDefaultInputDevice returned 0x%x\n", defaultInputDevice);
        err = defaultInputDevice;
        goto pa_error;
    }
    
    PaStreamParameters inputParams;
    inputParams.device = defaultInputDevice;
    inputParams.channelCount = 1;
    inputParams.sampleFormat = paFloat32;
    inputParams.suggestedLatency = Pa_GetDeviceInfo(defaultInputDevice)->defaultLowInputLatency;
    inputParams.hostApiSpecificStreamInfo = NULL;
    err = Pa_OpenStream(&stream,
                        &inputParams,
                        NULL,
                        SAMPLE_RATE,
                        FRAMES_PER_BUFFER,
                        paNoFlag,
                        &customAudioCallback,
                        &data);
    if (err != paNoError) goto pa_error;

    err = Pa_StartStream(stream);
    if (err != paNoError) goto pa_error;

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
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_1: changeView(loop_args, VIEW_INPUT); break;
                    case SDLK_2: changeView(loop_args, VIEW_SPECTRUM); break;
                    case SDLK_3: changeView(loop_args, VIEW_AUTOCORRELATION); break;
                    case SDLK_4: changeView(loop_args, VIEW_VOWEL_PREDICTION); break;
                    default: break; // Handle unrecognized input if necessary
                }
            }
        }
        copySamplesInOrder(&data, orderedData);
        if (currentView & VIEW_SPECTRUM){
            updateFFTData(data.samples, precomputed_hamming, windowed_data, fft_data, spectrum, SAMPLE_COUNT, fft_plan);
            smoothSpectrum(spectrum, smoothed_spectrum, SAMPLE_COUNT, 10);
            detectPeaks(smoothed_spectrum, SAMPLE_COUNT / 2 + 1, spectrum_peaks, max_peaks_spectrum, 15, 0.0f);
        }
        if (currentView & VIEW_AUTOCORRELATION){
            autocorrelation(orderedData, autocorr, SAMPLE_COUNT);
            detectPeaks(autocorr, SAMPLE_COUNT, autocorr_peaks, max_peaks_autocorr, 15, 0.0f);
        }
        if (currentView & VIEW_VOWEL_PREDICTION){
            energy = calculate_energy(orderedData, SAMPLE_COUNT);
            if (energy > energy_threshold) {
                vowel_prediction = 1;
                float* formants = calculate_formants(spectrum_peaks, autocorr_peaks, max_peaks_spectrum, max_peaks_autocorr, 2, SAMPLE_RATE, SAMPLE_COUNT);
                char* result = predict_vowel(formants, 2, 3, training_data, training_labels, 59);
                printf("Formant 1 : %f, Formant 2 : %f, Predicted label : %s\n", formants[0], formants[1], result);
                strncpy(predicted_label, result, 2);
                free(formants);
            }
            else {
                vowel_prediction = 0;
            }
        }
        pthread_mutex_lock(&globalDataLock);
        loop(loop_args);
        pthread_mutex_unlock(&globalDataLock);

        usleep((int)1e6 / 60); // Update at ~60 frames per second
    }

    err = Pa_StopStream(stream);
    if (err != paNoError) goto pa_error;

    err = Pa_CloseStream(stream);
    if (err != paNoError) goto pa_error;

    Pa_Terminate();

    pthread_mutex_destroy(&globalDataLock);
    fftwf_destroy_plan(fft_plan);

    if (spectrum) {
        free(spectrum);
        spectrum = NULL;
    }
    if (fft_data) {
        fftwf_free(fft_data);
        fft_data = NULL;
    }
    if (smoothed_spectrum) {
        free(smoothed_spectrum);
        smoothed_spectrum = NULL;
    }
    if (precomputed_hamming) {
        free(precomputed_hamming);
        precomputed_hamming = NULL;
    }
    if (autocorr) {
        free(autocorr);
        autocorr = NULL;
    }
    if (windowed_data) {
        free(windowed_data);
        windowed_data = NULL;
    }
    if (orderedData) {
        free(orderedData);
        orderedData = NULL;
    }
    if (data.samples) {
        free(data.samples);
        data.samples = NULL;
    }

    fftwf_cleanup();

    if (font != NULL) {
        TTF_CloseFont(font);
    }
    if (titleFont != NULL) {
        TTF_CloseFont(titleFont);
    }
    if (legendFont != NULL) {
        TTF_CloseFont(legendFont);
    }
    TTF_Quit();
    SDL_DestroyRenderer(main_renderer);
    SDL_DestroyWindow(main_window);
    SDL_Quit();

    return 0;
pa_error:
    Pa_Terminate();

    pthread_mutex_destroy(&globalDataLock);
    fftwf_destroy_plan(fft_plan);

    if (spectrum) free(spectrum);
    if (fft_data) fftwf_free(fft_data);
    if (smoothed_spectrum) free(smoothed_spectrum);
    if (precomputed_hamming) free(precomputed_hamming);
    if (autocorr) free(autocorr);
    if (spectrum_peaks) free(spectrum_peaks);
    if (autocorr_peaks) free(autocorr_peaks);
    if (windowed_data) free(windowed_data);
    if (orderedData) free(orderedData);
    if (data.samples) free(data.samples);

    fftwf_cleanup();

    
    if (font != NULL) TTF_CloseFont(font);
    if (titleFont != NULL) TTF_CloseFont(titleFont);
    if (legendFont != NULL) TTF_CloseFont(legendFont);
    TTF_Quit();
    SDL_DestroyRenderer(main_renderer);
    SDL_DestroyWindow(main_window);
    SDL_Quit();

    return err;
}
// if windows, main_function is WinMain, else main_function is main 
#ifdef _WIN32
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    return main_function();
}
#else
int main(int argc, char* argv[]) {
    return main_function();
}
#endif