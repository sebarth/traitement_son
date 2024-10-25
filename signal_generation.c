#include "signal_generation.h"

void signal_init(fftwf_complex* freq_domain, float* time_domain, float freq) {    
    fftwf_plan plan = fftwf_plan_dft_c2r_1d(SAMPLE_COUNT, freq_domain, time_domain, FFTW_ESTIMATE);
    
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        freq_domain[i][0] = 0.0f;
        freq_domain[i][1] = 0.0;
    }

    // Find the index of the 1500 Hz bin
    int bin = (int)((freq * SAMPLE_COUNT) / SAMPLE_RATE);
    freq_domain[bin][0] = SAMPLE_COUNT / 2.0f;
    freq_domain[bin][1] = 0.0f;

    // Compute the inverse FFT
    fftwf_execute(plan);

    // Normalize the output of the IFFT
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        time_domain[i] /= SAMPLE_COUNT;  // FFTW does not normalize automatically
    }
}

float generated_signal(float t, float* time_domain){
    int index = (int)((t * SAMPLE_RATE))%SAMPLE_COUNT;
    return time_domain[index];
}

void updateData(updateArgs args){
    // Update data variable
    for (unsigned long i = 0; i < FRAMES_PER_BUFFER; i++) {
            args.data->samples[args.data->currentIndex] = generated_signal(*(args.t), args.time_domain);
            args.data->currentIndex = (args.data->currentIndex + 1) % args.data->maxFrameIndex;
            *(args.t) += 1.0 / SAMPLE_RATE;
            if (*(args.t) >= 1.0f) *(args.t) -= 1.0f;
    }
}

void orderData(AudioData data, float* orderedData){
    // Copy first the last values (least recent)
    memcpy(orderedData, data.samples + data.currentIndex, (data.maxFrameIndex - data.currentIndex) * sizeof(float));
    // Then copy the first values until currentIndex (because currentIndex is the most recent)
    memcpy(orderedData + data.maxFrameIndex - data.currentIndex, data.samples, (data.currentIndex) * sizeof(float));
}

void* updateDataPtr(void* update_args){
    updateArgs args = *(updateArgs*)update_args;

    while (!*(args.quit1) || !*(args.quit2)) {
        pthread_mutex_lock(args.globalDataLock);
        updateData(args);
        orderData(*(args.data), args.orderedData);
        updateFFTData(args.fft_data, args.spectrum, SAMPLE_COUNT, args.fft_plan);
        pthread_mutex_unlock(args.globalDataLock);

        usleep(1000000 / (SAMPLE_RATE * FRAMES_PER_BUFFER));
    }
    return NULL;
}