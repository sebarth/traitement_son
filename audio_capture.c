//TODO tester le code un jour quand j'aurai un micro

#include <stdio.h>
#include <stdlib.h>
#include <portaudio.h>

#define SAMPLE_RATE  44100
#define FRAMES_PER_BUFFER 256

typedef struct {
    float *samples;
    int maxFrameIndex;  // Capacité du tableau, ici pour 5 secondes
    int currentIndex;   // L'indice pour écrire les nouveaux échantillons
} AudioData;

static int audioCallback(const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData) {
    AudioData *data = (AudioData*)userData;
    const float *in = (const float*)inputBuffer;

    if (inputBuffer == NULL) {
        for (unsigned long i = 0; i < framesPerBuffer; i++) {
            printf("No Input\n");
        }
    } else {
        for (unsigned long i = 0; i < framesPerBuffer; i++) {
            // Stocker l'échantillon dans le tampon circulaire
            data->samples[data->currentIndex] = *in++;
            
            // Incrémenter l'indice circulaire
            data->currentIndex = (data->currentIndex + 1) % data->maxFrameIndex;
        }
    }
    return paContinue;
}

void copySamplesInOrder(AudioData *data, float *orderedSamples) {
    int start = data->currentIndex;  // Point de départ du tampon circulaire
    int size = data->maxFrameIndex;

    // Copier les données dans le bon ordre
    for (int i = 0; i < size; i++) {
        orderedSamples[i] = data->samples[(start + i) % size];
    }
}

int main(void) {
    PaStream *stream;
    PaError err;
    AudioData data;

    // Le tableau doit contenir 5 secondes d'échantillons
    data.maxFrameIndex = SAMPLE_RATE * 5;  // 5 secondes d'échantillons
    data.samples = (float*)malloc(sizeof(float) * data.maxFrameIndex);
    data.currentIndex = 0;  // Initialiser à 0

    // Tampon temporaire pour stocker les données dans l'ordre
    float *orderedSamples = (float*)malloc(sizeof(float) * data.maxFrameIndex);

    // Initialisation de PortAudio
    err = Pa_Initialize();
    if (err != paNoError) goto error;

    // Ouvrir le flux audio
    err = Pa_OpenDefaultStream(&stream,
                               1,          // Canaux d'entrée (mono)
                               0,          // Pas de sortie
                               paFloat32,  // Format 32 bits float
                               SAMPLE_RATE,
                               FRAMES_PER_BUFFER,
                               audioCallback,
                               &data);
    if (err != paNoError) goto error;

    // Démarrer le flux
    err = Pa_StartStream(stream);
    if (err != paNoError) goto error;

    printf("Enregistrement en cours... Appuyez sur Entrée pour quitter\n");
    
    // Continue d'enregistrer jusqu'à l'appui sur Entrée
    getchar();

    // Arrêter le flux
    err = Pa_StopStream(stream);
    if (err != paNoError) goto error;

    err = Pa_CloseStream(stream);
    if (err != paNoError) goto error;

    Pa_Terminate();

    // Copier les données dans le bon ordre
    copySamplesInOrder(&data, orderedSamples);

    // Tu peux maintenant utiliser `orderedSamples` pour générer le graphique
    printf("Les 5 dernières secondes d'audio dans le bon ordre :\n");
    for (int i = 0; i < data.maxFrameIndex; i++) {
        printf("%f\n", orderedSamples[i]);
    }

    free(data.samples);
    free(orderedSamples);
    return 0;

error:
    Pa_Terminate();
    fprintf(stderr, "Une erreur est survenue: %s\n", Pa_GetErrorText(err));
    return -1;
}
