#pragma once

#include <cstdint>
#include <SDL2/SDL.h>

// Platform: the thin layer between the pure interpreter and the OS.
// Wraps SDL2 (or your library of choice) so Chip8 stays portable.
// Three responsibilities:
//   1. Create a window + renderer + texture, draw the 64x32 framebuffer
//      scaled up (e.g. 10x or 15x).
//   2. Pump the event queue, translate key events into the 16-key hex
//      keypad array Chip8 expects.
//   3. Optionally: play a beep while soundTimer > 0.
//
// Keep the Chip8 class free of any SDL include. That separation is the
// whole point of this file.

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

class Platform {
public:


    Platform(const char* title, int windowW, int windowH, int textureW, int textureH);
    ~Platform();

    // Upload the 64x32 buffer to the GPU texture and present.
    // `buffer` points to DISPLAY_W * DISPLAY_H bytes, each 0 or 1.
    // You'll expand each byte to a 32-bit pixel (0xFFFFFFFF or 0x000000FF).
    void render(const uint8_t* buffer, int pitchPixels);

    // Poll SDL events. Write 0/1 into `keys[16]`. Return false if the user
    // closed the window — main loop should exit.
    bool processInput(uint8_t* keys);

private:
    SDL_Window*   window   = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture*  texture  = nullptr;
};
