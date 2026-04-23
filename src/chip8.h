#pragma once

#include <cstdint>
#include <array>
#include <string>
#include <iostream>
#include <fstream>
#include <random>

// Chip8: the virtual machine itself.
// Owns memory, registers, stack, timers, display buffer, and keypad state.
// Knows nothing about SDL, windows, or the OS. A pure interpreter you can
// step one instruction at a time. Rendering and input are handled elsewhere
// and communicate through the public display buffer and keypad arrays.
class Chip8 {
public:
    // CHIP-8 spec constants. Look these up — don't just trust them.
    static constexpr unsigned MEM_SIZE     = 4096;   // 4 KB RAM
    static constexpr unsigned NUM_REGS     = 16;     // V0..VF
    static constexpr unsigned STACK_SIZE   = 16;
    static constexpr unsigned DISPLAY_W    = 64;
    static constexpr unsigned DISPLAY_H    = 32;
    static constexpr unsigned KEY_COUNT    = 16;
    static constexpr unsigned ROM_START    = 0x200;  // Programs load here
    static constexpr unsigned FONTSET_ADDR = 0x50;   // Conventional location

    Chip8();

    // Reset state, copy fontset into memory, set PC = 0x200.
    void reset();

    // Load a ROM file into memory starting at ROM_START.
    // Returns false on I/O error or oversized ROM.
    bool loadRom(const std::string& path);

    // Fetch-decode-execute one instruction. Advances PC.
    void cycle();

    // Call at 60 Hz. Decrements delay + sound timers if non-zero.
    void tickTimers();

    // Exposed for the platform layer to read/write. Keeping them public is
    // fine for a small project — getters/setters would just add noise.
    std::array<uint8_t, MEM_SIZE>                memory{};
    std::array<uint8_t, NUM_REGS>                V{};        // General regs
    uint16_t                                     I{0};       // Index register
    uint16_t                                     pc{0};      // Program counter
    std::array<uint16_t, STACK_SIZE>             stack{};
    uint8_t                                      sp{0};      // Stack pointer
    uint8_t                                      delayTimer{0};
    uint8_t                                      soundTimer{0};
    std::array<uint8_t, DISPLAY_W * DISPLAY_H>   display{};  // 0 or 1 per px
    std::array<uint8_t, KEY_COUNT>               keys{};     // 0 = up, 1 = down
    bool                                         drawFlag{false};

private:
    // Decode the current 16-bit opcode and dispatch to the handler.
    // You will likely implement this as a big switch on the high nibble,
    // with sub-switches for 0x0, 0x8, 0xE, 0xF families.
    void executeOpcode(uint16_t opcode);
};
