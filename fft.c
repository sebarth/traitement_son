#include "fft.h"

void fft_init(int size, float* in, fftwf_complex* out, char* file, fftwf_plan* plan) {
    /*
    Initializes the fft plan and imports wisdom from a file if provided.
    size : the size of the input array
    in : the input array
    out : the output array
    file : the file to import wisdom from, if NULL, no wisdom is imported
    */    
    if (file != NULL) fftwf_import_wisdom_from_filename(file);
    *plan = fftwf_plan_dft_r2c_1d(size, in, out, FFTW_ESTIMATE);
}

void updateFFTData(fftwf_complex* fft_data, float* spectrum, int size, fftwf_plan plan) {
    /*
    Updates the fft data and the spectrum.
    fft_data : the fft data that is updated
    spectrum : the spectrum that is updated
    size : the size of the input array
    plan : the fft plan to execute
    */
    fftwf_execute(fft_plan);

    // update spectrum
    for (int i = 0; i < size; i++){
        spectrum[i] = sqrt(fft_data[i][0] * fft_data[i][0] + fft_data[i][1] * fft_data[i][1]); // magnitude
    }
}