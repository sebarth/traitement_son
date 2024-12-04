#include "math_tools.h"

void fft_init(int size, float* in, fftwf_complex* out, char* file, fftwf_plan* plan) {
    // if (file != NULL) { 
    //     fftwf_import_wisdom_from_filename(file);
    // }
    *plan = fftwf_plan_dft_r2c_1d(size, in, out, FFTW_ESTIMATE);
    // fftwf_export_wisdom_to_filename(file);
}

float fast_log10f(float x) {
    union { float f; int i; } vx = { x };
    union { int i; float f; } mx = { (vx.i & 0x007FFFFF) | 0x3f000000 };
    float y = vx.i;
    y *= 1.1920928955078125e-7f;

    return y - 124.22551499f
        - 1.498030302f * mx.f
        - 1.72587999f / (0.3520887068f + mx.f);
}

void updateFFTData(float* data, float* hamming, float* windowed_data, fftwf_complex* fft_data, float* spectrum, int size, fftwf_plan plan) {
    // Hamming windowing
    for (int i = 0; i < size; i++) {
        windowed_data[i] = data[i] * hamming[i];
    }
    fftwf_execute(fft_plan);

    // update spectrum
    for (int i = 0; i < size; i++){
        spectrum[i] = sqrt((float) fft_data[i][0] * fft_data[i][0] + (float) fft_data[i][1] * fft_data[i][1]); // magnitude
        // scaling
        spectrum[i] /= size;
        /*if (spectrum[i] >= NOISE_FLOOR) {
            spectrum[i] = 20 * fast_log10f(spectrum[i]);
        } else {
            spectrum[i] = 20 * LOG_NOISE_FLOOR;
        }*/
    }
}

void smoothSpectrum(float* spectrum, float* smoothed, int size, int window_size) {
    int half_window = window_size / 2;
    for (int i = 0; i < size; i++) {
        smoothed[i] = 0.0f;
        int count = 0;
        for (int j = -half_window; j <= half_window; j++) {
            if (i + j >= 0 && i + j < size) {
                smoothed[i] += spectrum[i + j];
                count++;
            }
        }
        smoothed[i] /= count;
    }
}