#include "platform.h"

// You'll implement this with SDL2. The standard pattern:
//
//   SDL_Init(SDL_INIT_VIDEO)
//   SDL_CreateWindow(title, centered, centered, windowW, windowH, 0)
//   SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED)
//   SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
//                     SDL_TEXTUREACCESS_STREAMING, textureW, textureH)
//
// render():
//   Convert the 0/1 byte buffer into a uint32_t[64*32] pixel buffer
//   (0xFFFFFFFF for on, 0x000000FF for off), then
//   SDL_UpdateTexture -> SDL_RenderClear -> SDL_RenderCopy -> SDL_RenderPresent.
//
// processInput():
//   SDL_PollEvent loop. The conventional CHIP-8 keymap is:
//     1 2 3 4        ->  1 2 3 C
//     Q W E R        ->  4 5 6 D
//     A S D F        ->  7 8 9 E
//     Z X C V        ->  A 0 B F
//   Look up this mapping — it's universal across emulators.
//
// Destructor: destroy texture, renderer, window; SDL_Quit.

Platform::Platform(const char* title, int windowW, int windowH, int textureW, int textureH) {
    
    SDL_Init(SDL_INIT_VIDEO);

    SDL_CreateWindowAndRenderer(windowW * 8 , windowH * 8, 0, &window, &renderer);
    SDL_RenderSetScale(renderer,8,8);

    // SDL_SetRenderDrawColor(renderer, 0,0,0,255);
    // SDL_RenderClear(renderer);

    // SDL_SetRenderDrawColor(renderer,255,255,255,255);


    // SDL_RenderPresent(renderer);
    

}

Platform::~Platform(){
    SDL_Quit();
    
}

void Platform::render(const uint8_t* buffer, int pitchPixels){

    

    for (int y = 0; y < 32; y++){
        for (int x = 0; x < 64; x++){
            uint8_t pixel = buffer[y * pitchPixels + x];
            if (pixel == 1){
                SDL_SetRenderDrawColor(renderer,255,255,255,255);
                SDL_RenderDrawPoint(renderer,x,y);
            } else {
                SDL_SetRenderDrawColor(renderer,0,0,0,255);
                SDL_RenderDrawPoint(renderer,x,y);
            }
        }
    }
    SDL_RenderPresent(renderer);

    
}

bool Platform::processInput(uint8_t* keys){
    SDL_Event e;
    bool open = true;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) open = false;
        if(e.type == SDL_KEYDOWN){
            switch (e.key.keysym.sym) {
                case SDLK_1: keys[0x1] = 1; break;
                case SDLK_2: keys[0x2] = 1; break;
                case SDLK_3: keys[0x3] = 1; break;
                case SDLK_4: keys[0xC] = 1; break;
                case SDLK_q: keys[0x4] = 1; break;
                case SDLK_w: keys[0x5] = 1; break;
                case SDLK_e: keys[0x6] = 1; break;
                case SDLK_r: keys[0xD] = 1; break;
                case SDLK_a: keys[0x7] = 1; break;
                case SDLK_s: keys[0x8] = 1; break;
                case SDLK_d: keys[0x9] = 1; break;
                case SDLK_f: keys[0xE] = 1; break;
                case SDLK_z: keys[0xA] = 1; break;
                case SDLK_x: keys[0x0] = 1; break;
                case SDLK_c: keys[0xB] = 1; break;
                case SDLK_v: keys[0xF] = 1; break;
            }
        }
        if(e.type == SDL_KEYUP){
            switch (e.key.keysym.sym) {
                case SDLK_1: keys[0x1] = 0; break;
                case SDLK_2: keys[0x2] = 0; break;
                case SDLK_3: keys[0x3] = 0; break;
                case SDLK_4: keys[0xC] = 0; break;
                case SDLK_q: keys[0x4] = 0; break;
                case SDLK_w: keys[0x5] = 0; break;
                case SDLK_e: keys[0x6] = 0; break;
                case SDLK_r: keys[0xD] = 0; break;
                case SDLK_a: keys[0x7] = 0; break;
                case SDLK_s: keys[0x8] = 0; break;
                case SDLK_d: keys[0x9] = 0; break;
                case SDLK_f: keys[0xE] = 0; break;
                case SDLK_z: keys[0xA] = 0; break;
                case SDLK_x: keys[0x0] = 0; break;
                case SDLK_c: keys[0xB] = 0; break;
                case SDLK_v: keys[0xF] = 0; break;
            }
        }
    }
    return open;
}