#include "rendering.h"

void init(graphBoundaries* boundaries1, graphBoundaries* boundaries2, Button* changeWindowButton, loopArgs args){
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    main_window = SDL_CreateWindow("Amplitude Graph", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    
    main_renderer = SDL_CreateRenderer(main_window, -1, SDL_RENDERER_PRESENTVSYNC);

    SDL_RWops* rw_RobotoLight = SDL_RWFromMem(fonts_Roboto_Light_ttf, fonts_Roboto_Light_ttf_len);
    args.font = TTF_OpenFontRW(rw_RobotoLight, 1, 16);

    SDL_RWops* rw_RobotoRegular = SDL_RWFromMem(fonts_Roboto_Regular_ttf, fonts_Roboto_Regular_ttf_len);
    args.buttonFont = TTF_OpenFontRW(rw_RobotoRegular, 1, 16);

    SDL_RWops* rw_RobotoItalic = SDL_RWFromMem(fonts_Roboto_Italic_ttf, fonts_Roboto_Italic_ttf_len);
    args.legendFont = TTF_OpenFontRW(rw_RobotoItalic, 1, 16);

    SDL_SetRenderDrawColor(main_renderer, 255, 255, 255, 255); // Set to white background
    SDL_RenderClear(main_renderer);
    SDL_SetRenderTarget(main_renderer, NULL);

    boundaries1->xInterval.min = -(float)SAMPLE_COUNT / SAMPLE_RATE;
    boundaries1->xInterval.max = 0.0f;
    boundaries1->yInterval.min = -1.0f;
    boundaries1->yInterval.max = 1.0f;
    
    boundaries2->xInterval.min = 0.0f;
    //using f_k = k.F_s/N
    //max val of k : SAMPLE_COUNT/2 --> SAMPLE_COUNT / 2 * SAMPLE_RATE / SAMPLE_COUNT = SAMPLE_RATE / 2
    boundaries2->xInterval.max = SAMPLE_RATE / 2.0f;
    boundaries2->yInterval.min = 0.0f;
    boundaries2->yInterval.max = 1.0f;

    changeWindowButton->rect = (SDL_Rect){WIDTH / 2 - 75, HEIGHT - MARGIN_HEIGHT + 50, 150, 25};
    //grey color
    changeWindowButton->bgColor = (SDL_Color){200, 200, 200, 255};
    //darker grey color
    changeWindowButton->hoverColor = (SDL_Color){175, 175, 175, 255};
    changeWindowButton->text = "Switch Graph";
    changeWindowButton->textColor = (SDL_Color){0, 0, 0, 255};
    changeWindowButton->isHovered = false;
    changeWindowButton->onClick = &onButtonClick;
}

void renderButton(SDL_Renderer* renderer, Button* changeWindowButton, TTF_Font* font) {
    SDL_Color bgColor;
    if (changeWindowButton->isHovered) {
        bgColor = changeWindowButton->hoverColor;
    } else {
        bgColor = changeWindowButton->bgColor;
    }
    roundedBoxRGBA(renderer, changeWindowButton->rect.x, changeWindowButton->rect.y, changeWindowButton->rect.x + changeWindowButton->rect.w, changeWindowButton->rect.y + changeWindowButton->rect.h, 12, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
	roundedRectangleRGBA(renderer, changeWindowButton->rect.x, changeWindowButton->rect.y, changeWindowButton->rect.x + changeWindowButton->rect.w, changeWindowButton->rect.y + changeWindowButton->rect.h, 12, 0, 0, 0, 255);
	int text_height = 0;
    int text_width = 0;
    TTF_SizeText(font, changeWindowButton->text, &text_width, &text_height);
    drawText(renderer, font, changeWindowButton->text, changeWindowButton->rect.x + changeWindowButton->rect.w / 2 - text_width / 2, changeWindowButton->rect.y + changeWindowButton->rect.h / 2 - text_height / 2, changeWindowButton->textColor);
}

bool isMouseOverButton(Button *changeWindowButton, int mouseX, int mouseY) {
    return (mouseX >= changeWindowButton->rect.x && mouseX <= changeWindowButton->rect.x + changeWindowButton->rect.w && mouseY >= changeWindowButton->rect.y && mouseY <= changeWindowButton->rect.y + changeWindowButton->rect.h);
}

void onButtonClick(void* void_args) {
    loopArgs* args = (loopArgs*)void_args;
    if (args->currentWindow == 1) {
        args->currentWindow = 2;
        SDL_SetWindowTitle(main_window, "Spectrum Graph");
    } else {
        args->currentWindow = 1;
        SDL_SetWindowTitle(main_window, "Amplitude Graph");
    }
}

void loop(loopArgs args){
    if (args.currentWindow == 1){
        //update all boundaries to fit the graph
        //have to put min value (here 0.1f) or it bugs and doesn't display well (bc min = max so you divide by 0)
        (args.boundaries1)->yInterval.max = fmax(dataMax(args.orderedData, SAMPLE_COUNT), 0.1f);
        (args.boundaries1)->yInterval.min = fmin(dataMin(args.orderedData, SAMPLE_COUNT), -0.1f);

        SDL_SetRenderDrawColor(main_renderer, 255, 255, 255, 255);
        SDL_RenderClear(main_renderer);

        SDL_SetRenderDrawColor(main_renderer, args.color1.r, args.color1.g, args.color1.b, args.color1.a);
        drawGraph(main_renderer, args.orderedData, SAMPLE_COUNT, *(args.boundaries1), args.font, "Time (s)", "Amplitude (Arbitrary unit)");
        renderButton(main_renderer, args.changeWindowButton, args.buttonFont);
        
        SDL_RenderPresent(main_renderer);
    }
    else if (args.currentWindow == 2){
        (args.boundaries2)->yInterval.max = fmax(dataMax(args.spectrum, SAMPLE_COUNT / 2 + 1), 1.0f);
        (args.boundaries2)->yInterval.min = fmin(dataMin(args.spectrum, SAMPLE_COUNT / 2 + 1), 1.0f);

        SDL_SetRenderDrawColor(main_renderer, 255, 255, 255, 255);
        SDL_RenderClear(main_renderer);

        SDL_SetRenderDrawColor(main_renderer, args.color2.r, args.color2.g, args.color2.b, args.color2.a);
        drawGraph(main_renderer, args.spectrum, SAMPLE_COUNT / 2 + 1, *(args.boundaries2), args.font, "Frequency (Hz)", "Magnitude (dB)");
        renderButton(main_renderer, args.changeWindowButton, args.buttonFont);

        SDL_RenderPresent(main_renderer);
    }
}