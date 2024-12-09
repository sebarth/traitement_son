#ifndef RENDERING_H
#define RENDERING_H

#include "libs.h"
#include "graphs.h"
#include "constants.h"
#include "cross_platform.h"
#include "fonts.h"

/*
Button is a struct that represents a button on the screen.
rect : the place where the button is drawn
bgColor : the color of the button when it's not hovered
hoverColor : the color of the button when it's hovered
textColor : the color of the text on the button
text : the text on the button
isHovered : whether the mouse is over the button
onClick : the function pointer that is called when the button is clicked; 
is supposed to be a loopArgs pointer type, but it's not defined yet 
*/
typedef struct {
    SDL_Rect rect;
    SDL_Color bgColor;
    SDL_Color hoverColor;
    SDL_Color textColor;
    char* text;
    bool isHovered;
    void (*onClick)(void* args); // kinda ugly but loopArgs is not defined yet...
} Button;

typedef struct{
    graphBoundaries* boundaries1;
    graphBoundaries* boundaries2;
    graphBoundaries* boundaries3;
    AudioData* data;
    float* orderedData;
    fftwf_complex* fft_data;
    float* spectrum;
    float* autocorr;
    pthread_mutex_t* globalData;
    int currentWindow; // value is 1 or 2
    Button* changeWindowButton;
    SDL_Color color1;
    SDL_Color color2;
    TTF_Font* font;
    TTF_Font* titleFont;
    TTF_Font* legendFont;
    ViewType* currentView;
    int* spectrum_peaks;
    int* autocorr_peaks;
    int max_peaks_spectrum;
    int max_peaks_autocorr;
    char* predicted_label;
    int* vowel_prediction;
} loopArgs;

void init(graphBoundaries* boundaries1, graphBoundaries* boundaries2, Button* button, loopArgs* args);
void loop(loopArgs args);
void renderButton(SDL_Renderer* renderer, Button* button, TTF_Font* font);
bool isMouseOverButton(Button* button, int mouseX, int mouseY);
void onButtonClick(void* v_args);
void changeView(loopArgs args, ViewType view);

#endif // RENDERING_H