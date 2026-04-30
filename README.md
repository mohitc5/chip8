# CHIP-8 Interpreter

A CHIP-8 virtual machine written in C++, rendered with SDL2. CHIP-8 is an interpreted bytecode language from the late 1970s designed for 8-bit microcomputers: 35 opcodes, a 64×32 monochrome display, a 16-key hex keypad, 4 KB of RAM. This emulator runs the canonical test suite and plays original games like Pong and Tetris. The purpose of this project was for me to explore emulation development, and to eventually develope more emulators for retro consoles like the NES and Gameboy.

<img width="600" height="338" alt="Adobe Express - tetrischip8 (1)" src="https://github.com/user-attachments/assets/516fcd4e-2b57-464c-a69a-6273eadb94db" />

## Highlights

- **Full instruction set.** All 35 standard CHIP-8 opcodes, including DXYN sprite drawing with XOR collision detection, BCD conversion (FX33), and the load/store/shift behaviours that differ between CHIP-8 revisions.
- **Clean separation between VM and platform.** The `Chip8` class is a pure interpreter — it owns memory, registers, stack, timers, the framebuffer, and the keypad state, and knows nothing about SDL or windowing. A separate `Platform` class wraps SDL2 for rendering and input. The two communicate only through public buffers (`display[2048]`, `keys[16]`) and a `drawFlag`. The same `Chip8` could be wired to a terminal frontend, a web canvas, or a unit test without touching its source.
- **Decoupled CPU and timer clocks.** The CPU runs at a configurable rate; the delay and sound timers tick independently at exactly 60 Hz, as the spec requires. Rendering is only triggered by opcodes that actually modify the framebuffer (`00E0` and `DXYN`), not on every cycle.
- **Validated against the standard test ROMs.** Runs Timendus's CHIP-8 test suite (logo, IBM, corax+, flags, quirks, keypad) and the BC_test / test_opcode ROMs commonly used to verify emulator correctness.

<img width="1019" height="505" alt="image" src="https://github.com/user-attachments/assets/0c78c195-44ba-4322-91e4-bcd218e8b23f" />
<img width="1020" height="515" alt="image" src="https://github.com/user-attachments/assets/a1dec145-4231-4d2e-a773-86ac8f5942c3" />
<img width="1024" height="513" alt="image" src="https://github.com/user-attachments/assets/2f98448c-118b-471e-b16e-a5c26b2d5393" />

## Architecture

```
src/
├── chip8.h, chip8.cpp        # The virtual machine. Memory, registers, stack,
│                               timers, display buffer, keypad state, opcode
│                               dispatch. Zero OS dependencies.
├── platform.h, platform.cpp  # SDL2 wrapper. Window, renderer, event pump,
│                               keymap.
└── main.cpp                  # Entry point and main loop. Wires Chip8 and
                                Platform together; manages CPU/timer/render
                                clocks.
```

`Chip8::cycle()` fetches a 16-bit opcode from `memory[pc]`, advances `pc` by two, then dispatches through a switch on the high nibble (with sub-switches for the 0x0, 0x8, 0xE, and 0xF families). Drawing is XOR-based — any pixel that flips from on to off sets `VF = 1` so games can detect sprite collisions.

The fontset (5×4 sprites for hex digits 0–F) is copied into RAM at `0x50` on reset; ROMs are loaded starting at `0x200`, the conventional CHIP-8 program entry point. The PC is initialised to `0x200`.

## Build

Requires CMake ≥ 3.16, a C++17 compiler, and SDL2.

```bash
# Ubuntu / WSL
sudo apt install libsdl2-dev cmake

# macOS (Homebrew)
brew install sdl2 cmake
```

Then:

```bash
mkdir build && cd build
cmake ..
cmake --build .
./chip8
```

`main.cpp` currently loads `test_data/Tetris.ch8` by default. To try a different ROM, change the `loadRom(...)` call near the top of `main.cpp` and rebuild. Command-line argument parsing (`./chip8 <scale> <cycle-delay-ms> <rom-path>`) is on the roadmap.

## Controls

CHIP-8 uses a 16-key hex keypad. The conventional keyboard mapping is:

```
  CHIP-8 keypad           Keyboard
  ┌───┬───┬───┬───┐    ┌───┬───┬───┬───┐
  │ 1 │ 2 │ 3 │ C │    │ 1 │ 2 │ 3 │ 4 │
  ├───┼───┼───┼───┤    ├───┼───┼───┼───┤
  │ 4 │ 5 │ 6 │ D │    │ Q │ W │ E │ R │
  ├───┼───┼───┼───┤    ├───┼───┼───┼───┤
  │ 7 │ 8 │ 9 │ E │    │ A │ S │ D │ F │
  ├───┼───┼───┼───┤    ├───┼───┼───┼───┤
  │ A │ 0 │ B │ F │    │ Z │ X │ C │ V │
  └───┴───┴───┴───┘    └───┴───┴───┴───┘
```

## Test ROMs

`test_data/` contains the Timendus CHIP-8 test suite plus a few games. (not included in repo due to licensing)

| ROM                       | What it verifies                                          |
|---------------------------|-----------------------------------------------------------|
| `1-chip8-logo.ch8`        | DXYN sprite drawing and the basic frame loop              |
| `2-ibm-logo.ch8`          | Memory layout, jumps, fetch/decode                        |
| `3-corax+.ch8`            | Per-opcode correctness across the full instruction set    |
| `4-flags.ch8`             | Carry / borrow flag handling on arithmetic ops            |
| `5-quirks.ch8`            | Behavioural quirks (shift, jump-with-offset, FX55/65)     |
| `6-keypad.ch8`            | EX9E / EXA1 / FX0A keypad opcodes                         |
| `Pong.ch8`, `Tetris.ch8`  | Real games                                                |

## Notes on quirks

CHIP-8 has two flavours that disagree on a handful of opcodes — original COSMAC VIP behaviour vs. the later SCHIP/CHIP-48 variants. This implementation targets the COSMAC VIP behaviour: `8XY6`/`8XYE` shift `VY` into `VX`, `8XY1/2/3` reset `VF` to 0, and `FX55`/`FX65` increment the index register `I` after each load/store. Most original-era games (including Pong and Tetris) expect this.

## What I took away from building it

- Translating a hardware-style spec (registers, RAM-mapped fontset, BCD, XOR sprite drawing) into a clean software model.
- Why decoupling the interpreter from I/O matters in practice: the `Chip8` class can be stepped one instruction at a time from a unit test with no window, no SDL, no event loop.
- Driving multiple independent clocks (CPU, 60 Hz timers, render) from one loop without blocking on `sleep` and starving input.
- The opcode-level edge cases that the basic spec leaves under-specified — sprite wrap vs. clip, the shift quirk, the index-register increment in FX55/FX65 — which the Timendus quirks ROM surfaces directly.

## References

- [Cowgod's CHIP-8 Technical Reference](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM) — the canonical opcode-by-opcode spec.
- [Tobias V. Langhoff, *Guide to making a CHIP-8 emulator*](https://tobiasvl.github.io/blog/write-a-chip-8-emulator/) — clear walkthrough including the quirk tables.
- [Timendus CHIP-8 test suite](https://github.com/Timendus/chip8-test-suite) — the test ROMs in `test_data/`.
