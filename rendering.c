#include "rendering.h"

void init(graphBoundaries* boundaries1, graphBoundaries* boundaries2, Button* button){
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    main_window = SDL_CreateWindow("Amplitude Graph", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    
    main_renderer = SDL_CreateRenderer(main_window, -1, SDL_RENDERER_PRESENTVSYNC);

    font = TTF_OpenFont("fonts/Roboto-Light.ttf", 16);
    textFont = TTF_OpenFont("fonts/Roboto-Light.ttf", 24);

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

    button->rect = (SDL_Rect){WIDTH / 2 - 100, HEIGHT - MARGIN_HEIGHT + 50, 200, 75};
    //grey color
    button->color = (SDL_Color){200, 200, 200, 255};
    //darker grey color
    button->hoverColor = (SDL_Color){175, 175, 175, 255};
    button->text = "Switch Graph";
    button->isHovered = false;
    button->onClick = &onButtonClick;
}

void drawRoundedRect(SDL_Renderer *renderer, SDL_Rect rect, int radius, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    // Draw the central rectangle
    SDL_Rect centralRect = {rect.x + radius, rect.y, rect.w - 2 * radius, rect.h};
    SDL_RenderFillRect(renderer, &centralRect);

    // Draw the left and right rectangles
    SDL_Rect leftRect = {rect.x, rect.y + radius, radius, rect.h - 2 * radius};
    SDL_Rect rightRect = {rect.x + rect.w - radius, rect.y + radius, radius, rect.h - 2 * radius};
    SDL_RenderFillRect(renderer, &leftRect);
    SDL_RenderFillRect(renderer, &rightRect);

    // Draw the four corners
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w; // horizontal offset
            int dy = radius - h; // vertical offset
            if ((dx*dx + dy*dy) <= (radius * radius)) {
                SDL_RenderDrawPoint(renderer, rect.x + radius + dx, rect.y + radius + dy); // top-left
                SDL_RenderDrawPoint(renderer, rect.x + rect.w - radius + dx, rect.y + radius + dy); // top-right
                SDL_RenderDrawPoint(renderer, rect.x + radius + dx, rect.y + rect.h - radius + dy); // bottom-left
                SDL_RenderDrawPoint(renderer, rect.x + rect.w - radius + dx, rect.y + rect.h - radius + dy); // bottom-right
            }
        }
    }
}

void drawRoundedRectWithBorder(SDL_Renderer *renderer, SDL_Rect rect, int radius, SDL_Color fillColor, SDL_Color borderColor) {
    // Draw the filled rounded rectangle
    drawRoundedRect(renderer, rect, radius, fillColor);

    // Draw the border
    SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
   
    int numPoints = 50; // Number of points to use for the circle
    double angleStep = M_PI_2 / numPoints; // Step size for each point (90 degrees divided by numPoints)

    for (int i = 0; i <= numPoints; i++) {
        double angle = i * angleStep;
        int dx = (int)(radius * cos(angle));
        int dy = (int)(radius * sin(angle));
        
        // Draw the four corners
        SDL_RenderDrawPoint(renderer, rect.x + radius - dx, rect.y + radius - dy); // top-left
        SDL_RenderDrawPoint(renderer, rect.x + rect.w - radius + dx, rect.y + radius - dy); // top-right
        SDL_RenderDrawPoint(renderer, rect.x + radius - dx, rect.y + rect.h - radius + dy); // bottom-left
        SDL_RenderDrawPoint(renderer, rect.x + rect.w - radius + dx, rect.y + rect.h - radius + dy); // bottom-right
    }

    // Draw the horizontal and vertical lines
    SDL_RenderDrawLine(renderer, rect.x + radius, rect.y, rect.x + rect.w - radius, rect.y); // top
    SDL_RenderDrawLine(renderer, rect.x + radius, rect.y + rect.h, rect.x + rect.w - radius, rect.y + rect.h); // bottom
    SDL_RenderDrawLine(renderer, rect.x, rect.y + radius, rect.x, rect.y + rect.h - radius); // left
    SDL_RenderDrawLine(renderer, rect.x + rect.w, rect.y + radius, rect.x + rect.w, rect.y + rect.h - radius); // right
}

void renderButton(SDL_Renderer* renderer, Button* button) {
    SDL_Color color;
    if (button->isHovered) {
        color = button->hoverColor;
    } else {
        color = button->color;
    }
    drawRoundedRectWithBorder(main_renderer, button->rect, 10, color, (SDL_Color){0, 0, 0, 255});
    int text_height = 0;
    int text_width = 0;
    TTF_SizeText(font, button->text, &text_width, &text_height);
    drawText(renderer, font, button->text, button->rect.x + button->rect.w / 2 - text_width / 2, button->rect.y + button->rect.h / 2 - text_height / 2);
}

bool isMouseOverButton(Button *button, int mouseX, int mouseY) {
    return (mouseX >= button->rect.x && mouseX <= button->rect.x + button->rect.w &&
            mouseY >= button->rect.y && mouseY <= button->rect.y + button->rect.h);
}

void onButtonClick(void* v_args) {
    loopArgs* args = (loopArgs*)v_args;
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
        (args.boundaries1)->yInterval.max = maxAbs(dataMax(args.orderedData, SAMPLE_COUNT), 0.1f);
        (args.boundaries1)->yInterval.min = -args.boundaries1->yInterval.max;
        

        SDL_SetRenderDrawColor(main_renderer, 255, 255, 255, 255);
        SDL_RenderClear(main_renderer);

        SDL_SetRenderDrawColor(main_renderer, 255, 0, 0, 255);
        drawGraph(main_renderer, args.orderedData, SAMPLE_COUNT, *(args.boundaries1), font);
        renderButton(main_renderer, args.button);
        
        SDL_RenderPresent(main_renderer);
    }
    else if (args.currentWindow == 2){
        //update yInterval's max value to fit (min is always 0 so don't update min)
        //have to put min value (here 1.0f) or it bugs and doesn't display well (bc min = max so you divide by 0)
        (args.boundaries2)->yInterval.max = maxAbs(dataMax(args.spectrum, SAMPLE_COUNT / 2 + 1), 1.0f);

        SDL_SetRenderDrawColor(main_renderer, 255, 255, 255, 255);
        SDL_RenderClear(main_renderer);

        SDL_SetRenderDrawColor(main_renderer, 0, 0, 255, 255);
        drawGraph(main_renderer, args.spectrum, SAMPLE_COUNT / 2 + 1, *(args.boundaries2), font);
        renderButton(main_renderer, args.button);

        SDL_RenderPresent(main_renderer);
    }
}