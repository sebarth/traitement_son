#ifndef FFT_H
#define FFT_H

#include "libs.h"

extern fftwf_plan fft_plan;

void fft_init(int size, float* in, fftwf_complex* out, char* file, fftwf_plan* plan);
void updateFFTData(fftwf_complex* fft_data, float* spectrum, int size, fftwf_plan plan);

#endif // FFT_H