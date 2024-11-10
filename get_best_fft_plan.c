//! This file is only useful for getting the best fft plan, don't run it otherwise
#include <math.h>
#include <stdlib.h>
#include <fftw3.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

const int SAMPLE_COUNT = 1024;
const int RATE = 10000;
const int FRAMES_PER_BUFFER = 1;

fftwf_complex* freq_domain;
float* time_domain;

fftwf_plan plan;

typedef struct {
    float *samples;     //the data
    int maxFrameIndex;  //total number of indexes
    int currentIndex;   //next index to update
} AudioData;
typedef struct{
    AudioData* data;
    float* orderedData;
    float* t;
    fftwf_complex* fft_data;
    float* spectrum;
} updateArgs;

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
    plan = fftwf_plan_dft_r2c_1d(SAMPLE_COUNT, args.data->samples, args.fft_data, FFTW_PATIENT);
    char file[10] = "wisdom.txt";
    fftwf_export_wisdom_to_filename(file);
    fftwf_execute(plan);

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
}

void* updateDataPtr(void* update_args){
    updateArgs args = *(updateArgs*)update_args;

    //repeat 1000 times
    for (int i = 0; i < 1000; i++) {
        updateData(args);
        orderData(*(args.data), args.orderedData);
        updateFFTData(args);

        usleep(1000000 / (RATE * FRAMES_PER_BUFFER));
    }
    return NULL;
}

int main() {
    AudioData data;
    data.maxFrameIndex = SAMPLE_COUNT; // 5 secs
    data.samples = (float*)malloc(sizeof(float) * data.maxFrameIndex);
    data.currentIndex = 0;
    float *orderedData = (float*)malloc(sizeof(float) * data.maxFrameIndex);

    float t = 0;

    fftwf_complex *fft_data = (fftwf_complex*)fftwf_malloc(SAMPLE_COUNT * sizeof(fftwf_complex));
    float *spectrum = malloc(SAMPLE_COUNT * sizeof(float));

    signal_init();

    updateArgs update_args = {&data, orderedData, &t, fft_data, spectrum};

    pthread_t updateThread;

    pthread_create(&updateThread, NULL, updateDataPtr, &update_args);

    pthread_join(updateThread, NULL);

    free(data.samples);
    free(spectrum);
    fftwf_free(freq_domain);
    fftwf_free(fft_data);
    return 0;
}