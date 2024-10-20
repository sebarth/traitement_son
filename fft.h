#include <complex.h>
#include <math.h>

const int SAMPLE_COUNT = 1024;
const int RATE = 100;
const int LOG2_SAMPLE_COUNT = 10;

unsigned int reverse_bits(unsigned int x, int log2n) {
    unsigned int n = 0;
    for(int i = 0; i < log2n; i++) {
        n <<= 1;
        n |= (x & 1);
        x >>= 1;
    }
    return n;
}

void fft(double complex *x, int N, int log2n) {
    // Bit-reversal permutation
    for(unsigned int i = 0; i < N; i++) {
        unsigned int j = reverse_bits(i, log2n);
        if(j > i) {
            // Swap x[i] and x[j]
            double complex temp = x[i];
            x[i] = x[j];
            x[j] = temp;
        }
    }

    // FFT
    for (int s = 1; s <= log2(N); s++) {
        int m = 1 << s;
        int m2 = m >> 1;
        double theta = -2.0 * M_PI / m;  // Note the minus sign for forward FFT
        double complex wm = cexp(I * theta);
        
        for (int k = 0; k < N; k += m) {
            double complex w = 1.0 + 0.0 * I;
            for (int j = 0; j < m2; j++) {
                double complex t = w * x[k + j + m2];
                double complex u = x[k + j];
                x[k + j] = u + t;
                x[k + j + m2] = u - t;
                w *= wm;
            }
        }
    }
}