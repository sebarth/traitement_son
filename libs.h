#ifndef LIBS_H
#define LIBS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <fftw3.h>
#include <portaudio.h>

#ifndef SDL_LIBS
#define SDL_LIBS

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#endif // SDL_LIBS

#endif // LIBS_H