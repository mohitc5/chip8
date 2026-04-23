#include "chip8.h"
#include "platform.h"

#include <chrono>
#include <thread>
#include <cstdio>

// Entry point and main loop. Ties Chip8 + Platform together.
//
// Usage: chip8 <scale> <cycle-delay-ms> <rom-path>
//
// The main loop is the interesting bit. CHIP-8 has two separate clocks:
//   - CPU speed:   ~500-1000 Hz (configurable, game-dependent)
//   - Timer tick:  exactly 60 Hz (delay + sound timers)
//
// A simple correct pattern:
//
//   while (running) {
//       running = platform.processInput(chip8.keys.data());
//
//       auto now = clock::now();
//       if (now - lastCycle >= cycleDelay) {
//           lastCycle = now;
//           chip8.cycle();
//       }
//       if (now - lastTimer >= 16ms) {   // 60 Hz
//           lastTimer = now;
//           chip8.tickTimers();
//       }
//       if (chip8.drawFlag) {
//           platform.render(chip8.display.data(), DISPLAY_W);
//           chip8.drawFlag = false;
//       }
//   }
//
// Don't tie rendering to every cycle — only redraw when drawFlag is set
// (after a DXYN or 00E0 opcode).

int main(int argc, char** argv) {
    // TODO: parse argv
    // TODO: construct Platform and Chip8
    // TODO: chip8.loadRom(path)
    // TODO: main loop as above
    Chip8 chip;
    chip.loadRom("../test_data/Tetris.ch8");
    
    Platform platform("hi",64,32,0,0);
    platform.render(chip.display.data(),64);

    bool running = true;
    auto lastCycle = std::chrono::steady_clock::now();
    auto lastTimer = std::chrono::steady_clock::now();

    while (running) {
        running = platform.processInput(chip.keys.data());

        auto now = std::chrono::steady_clock::now(); // time now
        auto elapsedTimer = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTimer).count();
        auto elapsedCycle = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastCycle).count();
        
        if (elapsedCycle >= 1.5) {
            lastCycle = now;
            chip.cycle();
        }

        if (elapsedTimer >= 16) {   // 60 Hz
            lastTimer = now;
            chip.tickTimers();
        }
        if (chip.drawFlag) {
            platform.render(chip.display.data(), 64);
            chip.drawFlag = false;
        }
        
    }

    return 0;
}




// for (int i = 0; i < 200; i++){
//         chip.cycle();
//     }

//     for (int y = 0; y < 32; y++) {
//         for (int x = 0; x < 64; x++) {
//             std::cerr << (chip.display[y * 64 + x] ? '#' : ' ');
//             std::cerr << (chip.display[y * 64 + x]); //? '#' : ' ');
//         }
//         std::cerr << ('\n');
//     }