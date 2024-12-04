#ifndef FFT_H
#define FFT_H

#include "libs.h"
#include "constants.h"

extern fftwf_plan fft_plan;

void fft_init(int size, float* in, fftwf_complex* out, char* file, fftwf_plan* plan);
void updateFFTData(float* data, float* hamming, float* windowed_data, fftwf_complex* fft_data, float* spectrum, int size, fftwf_plan plan);
void smoothSpectrum(float* spectrum, float* smoothed, int size, int window_size);

#endif // FFT_H