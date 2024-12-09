#ifndef MATH_TOOLS_H
#define MATH_TOOLS_H

#include "libs.h"
#include "constants.h"

extern fftwf_plan fft_plan;

void fft_init(int size, float* in, fftwf_complex* out, char* file, fftwf_plan* plan);
void updateFFTData(float* data, float* hamming, float* windowed_data, fftwf_complex* fft_data, float* spectrum, int size, fftwf_plan plan);
void smoothSpectrum(float* spectrum, float* smoothed, int size, int window_size);
void autocorrelation(float* data, float* autocorr, int size);

/**
 * @brief Detects peaks in the given data array.
 *
 * This function scans through the provided data array and identifies peaks.
 * A peak is defined as an element that is greater than its immediate neighbors.
 * The indices of the detected peaks are stored in the `peaks` array.
 * If the number of detected peaks exceeds `max_peaks`, the function stops detecting further peaks.
 * If fewer peaks are detected than `max_peaks`, the remaining elements in the `peaks` array are set to -1.
 *
 * @param data Pointer to the array of float data to be analyzed.
 * @param size The size of the data array.
 * @param peaks Pointer to the array where the indices of the detected peaks will be stored.
 * @param max_peaks The maximum number of peaks to detect.
 * @param distance The minimum distance between two peaks.
 * @param min_height The minimum height of a peak.
 */
void detectPeaks(float* data, int size, int* peaks, int max_peaks, int distance, float min_height);

/**
 * @brief Predicts the label of a given data sample using the k-nearest neighbors (k-NN) algorithm.
 *
 * The function calculates the Euclidean distance between the input data sample and each training sample, 
 * identifies the k nearest neighbors, and determines the most common label among them. Labels are differentiated 
 * based on both vowel and gender (e.g., "aw" for a woman saying "a" and "am" for a man saying "a").
 *
 * @param data Pointer to the input data sample, which contains `size` float values (e.g., formants).
 * @param size The number of features in each data sample (e.g., 2 for two formants).
 * @param k The number of nearest neighbors to consider for the prediction.
 * @param training_data Pointer to the training dataset, where each training sample is a float array of size `size`.
 * @param training_labels Pointer to the labels corresponding to the training dataset, where each label is a string.
 * @param training_size The number of samples in the training dataset.
 * @return A pointer to the predicted label, which is the most common label among the k nearest neighbors. 
 *         The returned label is not dynamically allocated; it directly points to an element of `training_labels`.
 *
 * @note The function assumes that the `training_labels` remain valid throughout the lifetime of the returned pointer.
 *
 * Example usage:
 * @code
 * float data[2] = {500.0, 1500.0}; // Example formants
 * float* training_data[3] = { (float[]){600.0, 1400.0}, (float[]){550.0, 1600.0}, (float[]){490.0, 1520.0} };
 * char* training_labels[3] = { "aw", "am", "iw" };
 * int size = 2;
 * int k = 2;
 * int training_size = 3;
 *
 * char* result = predict_vowel(data, size, k, training_data, training_labels, training_size);
 * printf("Predicted label: %s\n", result);
 * @endcode
 */
char* predict_vowel(float* data, int size, int k, float (*training_data)[size], char** training_labels, int training_size);

/**
 * @brief Calculates the energy of a given data sample.
 *
 * The energy of a data sample is computed as the sum of the squares of its elements.
 *
 * @param data Pointer to the data sample.
 * @param size The number of elements in the data sample.
 * @return The energy of the data sample.
 */
float calculate_energy(float* data, int size);


/**
 * @brief Calculates the formants from the spectrum and autocorrelation peaks.
 *
 * This function calculates the formants from the spectrum peaks and autocorrelation peaks.
 * The formants are computed based on the reciprocal of the autocorrelation peaks.
 *
 * @param spectrum_peaks Pointer to the array of spectrum peaks.
 * @param autocorr_peaks Pointer to the array of autocorrelation peaks.
 * @param max_peaks_spectrum The maximum number of peaks to consider for the spectrum.
 * @param max_peaks_autocorr The maximum number of peaks to consider for the autocorrelation.
 * @param num_formants The number of formants to calculate.
 * @param sample_rate The sample rate of the audio data.
 * @param sample_count The number of samples in the data.
 * @return A pointer to an array of two formants.
 *
 * @note The returned pointer points to a dynamically allocated array of two formants.
 *       The caller should free the returned pointer.
 */
float* calculate_formants(int* spectrum_peaks, int* autocorr_peaks, int max_peaks_spectrum, int max_peaks_autocorr, int num_formants, int sample_rate, int sample_count);

#endif // MATH_TOOLS_H