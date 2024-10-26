#include "signal_generation.h"

void signal_init(fftwf_complex* freq_domain, float* time_domain, float freq) {    
    fftwf_plan plan = fftwf_plan_dft_c2r_1d(SAMPLE_COUNT, freq_domain, time_domain, FFTW_ESTIMATE);
    
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        freq_domain[i][0] = 0.0f;
        freq_domain[i][1] = 0.0f;
    }

    // Find the index of the 1500 Hz bin
    int bin = (int)(freq * SAMPLE_COUNT / SAMPLE_RATE);
    freq_domain[bin][0] = SAMPLE_COUNT / 2.0f;
    freq_domain[bin][1] = 0.0f;

    // Compute the inverse FFT
    fftwf_execute(plan);

    // Normalize the output of the IFFT
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        time_domain[i] /= SAMPLE_COUNT;  // FFTW does not normalize automatically
    }
    /* for (int i = 0; i < SAMPLE_COUNT; i++) {
        time_domain[i] = sinf(2 * M_PI * i / SAMPLE_RATE * freq);
    } */
}

float generated_signal(float t, float* time_domain){
    int index = (int)((t * SAMPLE_RATE))%SAMPLE_COUNT;
    return time_domain[index];
}

void updateData(updateArgs args){
void updateData(updateArgs args, struct timespec start_time){// Update data variable
    struct timespec current_time;
    clock_gettime(CLOCK_MONOTONIC, &current_time);
    double elapsed_time = (current_time.tv_sec - start_time.tv_sec) + (current_time.tv_nsec - start_time.tv_nsec) / 1e9;
    
    *args.t = (float)elapsed_time;
    int index = (int) (*(args.t) * SAMPLE_RATE);
    memcpy(args.data->samples + args.data->currentIndex, args.time_domain + index, FRAMES_PER_BUFFER * sizeof(float));
    args.data->currentIndex = (args.data->currentIndex + FRAMES_PER_BUFFER) % args.data->maxFrameIndex;
}

void orderData(AudioData data, float* orderedData){
    // Copy first the last values (least recent)
    memcpy(orderedData, data.samples + data.currentIndex, (data.maxFrameIndex - data.currentIndex) * sizeof(float));
    // Then copy the first values until currentIndex (because currentIndex is the most recent)
    memcpy(orderedData + data.maxFrameIndex - data.currentIndex, data.samples, (data.currentIndex) * sizeof(float));
}

void* updateDataPtr(void* update_args){
    updateArgs args = *(updateArgs*)update_args;
    struct timespec start_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    while (!*(args.quit1) || !*(args.quit2)) {
        pthread_mutex_lock(args.globalDataLock);
        updateData(args, start_time);
        orderData(*(args.data), args.orderedData);
        updateFFTData(args.fft_data, args.spectrum, SAMPLE_COUNT, fft_plan);
        pthread_mutex_unlock(args.globalDataLock);

        usleep(FRAMES_PER_BUFFER * (int)1e6 / (SAMPLE_RATE));
    }
    return NULL;
}