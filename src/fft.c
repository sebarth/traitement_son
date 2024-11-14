#include "fft.h"

void fft_init(int size, float* in, fftwf_complex* out, char* file, fftwf_plan* plan) {
    // if (file != NULL) { 
    //     fftwf_import_wisdom_from_filename(file);
    // }
    *plan = fftwf_plan_dft_r2c_1d(size, in, out, FFTW_ESTIMATE);
    // fftwf_export_wisdom_to_filename(file);
}

void updateFFTData(fftwf_complex* fft_data, float* spectrum, int size, fftwf_plan plan) {
    fftwf_execute(fft_plan);

    // update spectrum
    for (int i = 0; i < size; i++){
        spectrum[i] = sqrt(fft_data[i][0] * fft_data[i][0] + fft_data[i][1] * fft_data[i][1]); // magnitude
    }
}