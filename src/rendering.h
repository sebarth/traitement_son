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
    AudioData* data;
    float* orderedData;
    fftwf_complex* fft_data;
    float* spectrum;
    float* t;
    int* quit;
    pthread_mutex_t* globalData;
    int currentWindow; // value is 1 or 2
    Button* changeWindowButton;
    SDL_Color color1;
    SDL_Color color2;
} loopArgs;

void init(graphBoundaries* boundaries1, graphBoundaries* boundaries2, Button* button);
void loop(loopArgs args);
void renderButton(SDL_Renderer* renderer, Button* button);
bool isMouseOverButton(Button* button, int mouseX, int mouseY);
void onButtonClick(void* v_args);

#endif // RENDERING_H