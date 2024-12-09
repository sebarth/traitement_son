#include "rendering.h"

void init(graphBoundaries* boundaries1, graphBoundaries* boundaries2, Button* changeWindowButton, loopArgs *args){
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    main_window = SDL_CreateWindow("Graphe de l'amplitude", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    
    main_renderer = SDL_CreateRenderer(main_window, -1, SDL_RENDERER_PRESENTVSYNC);


    SDL_RWops* rw_RobotoLight = SDL_RWFromMem(fonts_Roboto_Light_ttf, fonts_Roboto_Light_ttf_len);
    args->font = TTF_OpenFontRW(rw_RobotoLight, 1, 16);
    if (args->font == NULL) {
        fprintf(stderr, "TTF_OpenFontRW: %s\n", TTF_GetError());
    }

    SDL_RWops* rw_RobotoRegular = SDL_RWFromMem(fonts_Roboto_Regular_ttf, fonts_Roboto_Regular_ttf_len);
    args->titleFont = TTF_OpenFontRW(rw_RobotoRegular, 1, 32);
    if (args->titleFont == NULL) {
        fprintf(stderr, "TTF_OpenFontRW: %s\n", TTF_GetError());
    }

    SDL_RWops* rw_RobotoItalic = SDL_RWFromMem(fonts_Roboto_Italic_ttf, fonts_Roboto_Italic_ttf_len);
    args->legendFont = TTF_OpenFontRW(rw_RobotoItalic, 1, 16);
    if (args->legendFont == NULL) {
        fprintf(stderr, "TTF_OpenFontRW: %s\n", TTF_GetError());
    }

    SDL_SetRenderDrawColor(main_renderer, 255, 255, 255, 255); // Set to white background
    SDL_RenderClear(main_renderer);
    SDL_SetRenderTarget(main_renderer, NULL);

    boundaries1->xInterval.min = -(float)SAMPLE_COUNT / SAMPLE_RATE * 1000;
    boundaries1->xInterval.max = 0.0f;
    boundaries1->yInterval.min = -1.0f;
    boundaries1->yInterval.max = 1.0f;
    
    boundaries2->xInterval.min = 0.0f;
    
    //Nyquist frequency
    boundaries2->xInterval.max = SAMPLE_RATE / 2.0f;
    boundaries2->yInterval.min = 0.0f;
    boundaries2->yInterval.max = 1.0f;
    
    (args->boundaries3)->xInterval.min = 0.0f;
    (args->boundaries3)->xInterval.max = (float) SAMPLE_COUNT / (float) SAMPLE_RATE * 1000;
    (args->boundaries3)->yInterval.min = 0.0f;
    (args->boundaries3)->yInterval.max = 1.0f;

    *args->currentView = VIEW_INPUT;

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
    if (*args->currentView == VIEW_INPUT) {
        *args->currentView = VIEW_SPECTRUM;
        SDL_SetWindowTitle(main_window, "Spectrum Graph");
    } else {
        *args->currentView = VIEW_INPUT;
        SDL_SetWindowTitle(main_window, "Amplitude Graph");
    }
}

void changeView(loopArgs args, ViewType view){
    *args.currentView = view;
    switch (*args.currentView)
    {
    case VIEW_INPUT:
        SDL_SetWindowTitle(main_window, "Graphe de l'amplitude");
        break;
    case VIEW_SPECTRUM:
        SDL_SetWindowTitle(main_window, "Graphe du spectre");
        break;
    case VIEW_AUTOCORRELATION:
        SDL_SetWindowTitle(main_window, "Graphe de l'autocorrélation");
        break;
    case VIEW_VOWEL_PREDICTION:
        SDL_SetWindowTitle(main_window, "Prédiction de voyelle");
        break;
    default:
        break;
    }
}

void render_view(SDL_Renderer* renderer, SDL_Color line_color, float* data, int data_count, graphBoundaries* bounds, TTF_Font* font, TTF_Font* titleFont, TTF_Font* legendFont, char* xLabel, char* yLabel, char* title, int withPeaks, int* peaks, int peak_count){
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    drawGraph(renderer, data, line_color, data_count, *bounds, font, titleFont, legendFont, xLabel, yLabel, title);

    if (withPeaks){
        drawPeaks(renderer, peaks, peak_count, data, data_count, *bounds, (SDL_Color){0, 0, 0, 255});
    }

    SDL_RenderPresent(renderer);
}

void input_view(loopArgs args) {
    float max = fmax(dataMax(args.orderedData, SAMPLE_COUNT), -dataMin(args.orderedData, SAMPLE_COUNT));
    (args.boundaries1)->yInterval.max = fmax(max, 0.1f);
    (args.boundaries1)->yInterval.min = fmin(-max, -0.1f);

    render_view(main_renderer, (SDL_Color){0, 0, 255, 0}, args.orderedData, SAMPLE_COUNT, args.boundaries1, args.font, args.titleFont, args.legendFont, "Temps (ms)", "Amplitude", "Signal audio", 0, NULL, 0);
}

void spectrum_view(loopArgs args) {
    (args.boundaries2)->yInterval.max = fmax(dataMax(args.spectrum, SAMPLE_COUNT / 2 + 1), 0.01f);
    render_view(main_renderer, (SDL_Color){255, 0, 0, 0}, args.spectrum, SAMPLE_COUNT / 2 + 1, args.boundaries2, args.font, args.titleFont, args.legendFont, "Fréquence (Hz)", "Amplitude", "Spectre de Fourier", 1, args.spectrum_peaks, args.max_peaks_spectrum);
}

void autocorrelation_view(loopArgs args){
    (args.boundaries3)->yInterval.max = fmax(dataMax(args.autocorr, SAMPLE_COUNT), 0.01f);
    (args.boundaries3)->yInterval.min = fmin(dataMin(args.autocorr, SAMPLE_COUNT), -0.01f);
    render_view(main_renderer, (SDL_Color){0, 255, 0, 0}, args.autocorr, SAMPLE_COUNT, args.boundaries3, args.font, args.titleFont, args.legendFont, "Retard (ms)", "Amplitude", "Autocorrélation", 1, args.autocorr_peaks, args.max_peaks_autocorr);
}

void vowel_prediction_view(loopArgs args){
    SDL_SetRenderDrawColor(main_renderer, 255, 255, 255, 255);
    SDL_RenderClear(main_renderer);
    //prediction of the vowel
    char string[32];
    if (*args.vowel_prediction) {
        snprintf(string, sizeof(string), "La voyelle prédite est : %c", args.predicted_label[0]);
    } else{
        snprintf(string, sizeof(string), "Je ne repère pas de voyelle.");
    }
    //get width and height of the text
    int width = 0;
    int height = 0;
    TTF_SizeText(args.titleFont, string, &width, &height);
    drawText(main_renderer, args.titleFont, string, (WIDTH - width) / 2, (HEIGHT - height) / 2, (SDL_Color){0, 0, 0, 255});
    
    SDL_RenderPresent(main_renderer);
}

void loop(loopArgs args){
    switch (*args.currentView)
    {
    case VIEW_INPUT:
        input_view(args);
        break;
    case VIEW_SPECTRUM:
        spectrum_view(args);
        break;
    case VIEW_AUTOCORRELATION:
        autocorrelation_view(args);
        break;
    case VIEW_VOWEL_PREDICTION:
        vowel_prediction_view(args);
        break;
    default:
        break;
    }
}