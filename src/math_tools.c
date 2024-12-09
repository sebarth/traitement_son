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
    for (int i = 0; i < size / 2 + 1; i++){
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
    int spectrum_size = size / 2 + 1;
    int half_window = window_size / 2;
    for (int i = 0; i < spectrum_size; i++) {
        smoothed[i] = 0.0f;
        int count = 0;
        for (int j = -half_window; j <= half_window; j++) {
            if (i + j >= 0 && i + j < spectrum_size) {
                smoothed[i] += spectrum[i + j];
                count++;
            }
        }
        smoothed[i] /= count;
    }
}

void autocorrelation(float* data, float* autocorr, int size) {
    for (int i = 0; i < size; i++) {
        autocorr[i] = 0.0f;
        for (int j = 0; j < size - i; j++) {
            autocorr[i] += data[j] * data[j + i];
        }
    }
}


void detectPeaks(float* data, int size, int* peaks, int max_peaks, int distance, float min_height) {
    float mean = 0.0f;
    for (int i = 0; i < size; i++) {
        mean += data[i];
    }
    mean /= size;
    int count = 0;
    for (int i = 1; i < size - 1; i++) {
        if (data[i] >= min_height && data[i] >= mean * 2 && data[i] > data[i - 1] && data[i] > data[i + 1]) {
            // Check if the peak is far enough from the previous peak
            if (count == 0 || i - peaks[count - 1] >= distance) {
                peaks[count++] = i;
                if (count >= max_peaks) {
                    break;
                }
            } else if (data[i] > data[peaks[count - 1]]) {
                // Replace the previous peak if the current one is higher
                peaks[count - 1] = i;
            }
        }
    }
    for (int i = count; i < max_peaks; i++) {
        peaks[i] = -1;
    }
}


char* predict_vowel(float* data, int size, int k, float (*training_data)[size], char** training_labels, int training_size) {
    // Allocate distances array
    float* distances = (float*)malloc(training_size * sizeof(float));

    // Calculate distances between the input data and training data
    for (int i = 0; i < training_size; i++) {
        float distance = 0.0f;
        for (int j = 0; j < size; j++) {
            float diff = data[j] - training_data[i][j];
            distance += diff * diff;
        }
        distances[i] = sqrtf(distance);
    }

    // Array to hold indices for sorting
    int* indices = (int*)malloc(training_size * sizeof(int));
    for (int i = 0; i < training_size; i++) {
        indices[i] = i;
    }

    // Sort distances and rearrange indices accordingly
    for (int i = 0; i < training_size; i++) {
        for (int j = i + 1; j < training_size; j++) {
            if (distances[indices[j]] < distances[indices[i]]) {
                int temp = indices[i];
                indices[i] = indices[j];
                indices[j] = temp;
            }
        }
    }

    // Count occurrences of the k nearest labels
    typedef struct {
        char* label;
        int count;
    } LabelCount;

    LabelCount* label_counts = (LabelCount*)malloc(k * sizeof(LabelCount));
    int unique_count = 0;

    for (int i = 0; i < k; i++) {
        char* label = training_labels[indices[i]];
        int found = 0;

        // Check if label already exists in label_counts
        for (int j = 0; j < unique_count; j++) {
            if (strcmp(label_counts[j].label, label) == 0) {
                label_counts[j].count++;
                found = 1;
                break;
            }
        }

        // If not found, add it to label_counts
        if (!found) {
            label_counts[unique_count].label = label;
            label_counts[unique_count].count = 1;
            unique_count++;
        }
    }

    // Find the most frequent label
    char* predicted_label = label_counts[0].label;
    int max_count = label_counts[0].count;

    for (int i = 1; i < unique_count; i++) {
        if (label_counts[i].count > max_count) {
            predicted_label = label_counts[i].label;
            max_count = label_counts[i].count;
        }
    }

    // Clean up
    free(distances);
    free(indices);
    free(label_counts);

    return predicted_label;
}

float calculate_energy(float* data, int size){
    float energy = 0;
    for (int i = 0; i < size; i++){
        energy += data[i] * data[i];
    }
    return energy;
}

float* calculate_formants(int* spectrum_peaks, int* autocorr_peaks, int max_peaks_spectrum, int max_peaks_autocorr, int num_formants, int sample_rate, int sample_count){
    float* formants = (float*)malloc(num_formants * sizeof(float));
    for (int i = 0; i < num_formants; i++){
        formants[i] = 0.0f;
    }

    int count = 0;
    for (int i = 0; i < max_peaks_spectrum; i++){
        int is_formant = 1;
        if (spectrum_peaks[i] == -1) break;
        for (int j = 0; j < max_peaks_autocorr; j++){
            if (autocorr_peaks[j] == -1) break;
            if (fabsf((float) spectrum_peaks[i] - (float) sample_rate / (float) autocorr_peaks[j]) < 1.0f){
                is_formant = 0;
                break;
            }
        }
        if (is_formant){
            formants[count++] = (float) spectrum_peaks[i] * (float) sample_rate / (float) sample_count;
            if (count >= num_formants) break;
        }
    }
    return formants;
}