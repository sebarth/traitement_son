#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "signal_generation.h"

void signal_init(fftwf_complex* freq_domain, float* time_domain, float freq) {
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        time_domain[i] = sinf(2 * M_PI * freq * i / SAMPLE_RATE);
    }
}

float signal(float t, float* time_domain) {
    int index = (int)((t * SAMPLE_RATE)) % SAMPLE_COUNT;
    return time_domain[index];
}

double updateData(updateArgs args) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    int index = (int)(*args.t * SAMPLE_RATE);
    memcpy(args.data->samples + args.data->currentIndex, args.time_domain + index, FRAMES_PER_BUFFER * sizeof(float));
    *args.t += (float)FRAMES_PER_BUFFER / (float)SAMPLE_RATE;
    args.data->currentIndex = (args.data->currentIndex + FRAMES_PER_BUFFER) % args.data->maxFrameIndex;
    clock_gettime(CLOCK_MONOTONIC, &end);
    long seconds = end.tv_sec - start.tv_sec;
    long nanoseconds = end.tv_nsec - start.tv_nsec;
    double elapsed = seconds + nanoseconds * 1e-9;
    return (double)*args.t;
}

double orderData(AudioData data, float* orderedData) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    memcpy(orderedData, data.samples + data.currentIndex, (data.maxFrameIndex - data.currentIndex) * sizeof(float));
    memcpy(orderedData + data.maxFrameIndex - data.currentIndex, data.samples, (data.currentIndex) * sizeof(float));
    clock_gettime(CLOCK_MONOTONIC, &end);
    long seconds = end.tv_sec - start.tv_sec;
    long nanoseconds = end.tv_nsec - start.tv_nsec;
    double elapsed = seconds + nanoseconds * 1e-9;
    return elapsed;
}

void* updateDataPtr(void* update_args) {
    updateArgs args = *(updateArgs*)update_args;
    while (!*(args.quit1) || !*(args.quit2)) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        pthread_mutex_lock(args.globalDataLock);
        double update_time = updateData(args);
        double orderData_time = orderData(*(args.data), args.orderedData);
        printf("\033[F\033[K");
        printf("Update time: %.9f seconds, OrderData time: %.9f seconds\n", update_time, orderData_time);
        pthread_mutex_unlock(args.globalDataLock);

        usleep(FRAMES_PER_BUFFER * (int)1e6 / (SAMPLE_RATE));
    }
    return NULL;
}