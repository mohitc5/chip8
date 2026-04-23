#include "chip8.h"

// Implementation lives here. You'll write:
//
//   Chip8::Chip8()            -> call reset()
//   Chip8::reset()            -> zero memory/regs, copy fontset to FONTSET_ADDR,
//                                set pc = ROM_START
//   Chip8::loadRom(path)      -> open binary, read into memory[ROM_START..]
//   Chip8::cycle()            -> fetch opcode (memory[pc] << 8 | memory[pc+1]),
//                                advance pc by 2, then executeOpcode(opcode)
//   Chip8::tickTimers()       -> decrement delayTimer/soundTimer if > 0
//   Chip8::executeOpcode(op)  -> the big switch. 35 opcodes total.
//
// The CHIP-8 fontset is a fixed 80-byte blob representing hex digits 0-F
// as 4x5 sprites. Every reference implementation has the same bytes —
// Cowgod's reference doc lists them. Put them in an anonymous namespace
// here (static const uint8_t fontset[80] = { ... }).


Chip8::Chip8(){
    reset();
}

void Chip8::reset(){
    memory.fill(0);
    V.fill(0);
    stack.fill(0);
    display.fill(0);
    keys.fill(0);
    I = 0;
    sp = 0;
    pc = ROM_START;

    static constexpr uint8_t fontset[80] = { 0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };
    std::copy(std::begin(fontset), std::end(fontset), memory.begin() + FONTSET_ADDR);
}


bool Chip8::loadRom(const std::string& path){

    std::ifstream file(path, std::ios::binary | std::ios::ate); // open in binary, set file read pointer to end

    std::streamsize rom_size = file.tellg();

    if (rom_size > 3584){
        std::cerr << "File too big!" ;
        return false; 
    }

    file.seekg(0, std::ios::beg); // send file pointer to beginning again

    file.read(reinterpret_cast<char*>(&memory[ROM_START]), rom_size); // reinterpret cast is for unrelated pointer types, (char*) chooses a type of cast on its own so might not work

    return true;
}


void Chip8::cycle(){

    // Fetch
    uint16_t opcode = (memory[pc] << 8) | memory[pc + 1]; // first byte shifted, second byte
    pc += 2;
    executeOpcode(opcode);
}

void Chip8::executeOpcode(uint16_t opcode){
    //std::cerr << std::hex << std::uppercase << opcode << "\n";
    switch(opcode & 0xF000){

        case 0x0000: // CLR or RST
            switch(opcode & 0x00FF){
                case 0x00E0:{ //CLR
                    display.fill(0);
                    drawFlag = true;
                    break;
                }
                case 0x00EE:{ // Return
                        //pop
                        sp--;
                        pc = stack[sp];
                    break;
                }
                default:
                    break;
            }
            break;
            
        case 0x1000:{ // 0x1NNN JUMP 
            uint16_t NNN = opcode & 0x0FFF;
            pc = NNN;
            break;
        }
        case 0x2000:{ // 0x2NNN CALL 
            uint16_t NNN = opcode & 0x0FFF;
            // push
            stack[sp] = pc;
            sp++;
            pc = NNN;
            break;
        }
        case 0x3000: {// 0x3XNN, Conditional Skip
            uint16_t X = (opcode & 0x0F00) >> 8;
            uint16_t NN = opcode & 0x00FF; 
            if (V[X] == NN){
                pc += 2;
            }
            break;
        }
        case 0x4000: {// 0x4XNN, Conditional Skip
            uint16_t X = (opcode & 0x0F00) >> 8;
            uint16_t NN = opcode & 0x00FF; 
            if (V[X] != NN){
                pc += 2;
            }
            break;
        }

        case 0x5000: {// 0x5XY0, Conditional Skip
            uint16_t X = (opcode & 0x0F00) >> 8;
            uint16_t Y = (opcode & 0x00F0) >> 4; 
            if (V[X] == V[Y]){
                pc += 2;
            }
            break;
        }

        
        
        case 0x6000: {// 0x6XNN, set register VX to NN
            uint16_t X = (opcode & 0x0F00) >> 8;
            V[X] = opcode & 0x00FF; 
            break;
        }
        case 0x7000: {// 0x7XNN, add NN to register VX  
            uint16_t X = (opcode & 0x0F00) >> 8;
            V[X] += opcode & 0x00FF; 
            break;
        }

        case 0x8000: {
            switch(opcode & 0x000F){
                case 0x0000:{ // 0x8XY0, Set
                    uint8_t X = (opcode & 0x0F00) >> 8;
                    uint8_t Y = (opcode & 0x00F0) >> 4; 
                    V[X] = V[Y];
                    break;
                }
                case 0x0001:{ // 0x8XY1, OR
                    uint8_t X = (opcode & 0x0F00) >> 8;
                    uint8_t Y = (opcode & 0x00F0) >> 4; 
                    V[X] =V[X] | V[Y];
                    V[0xF] = 0;
                    break;
                }
                case 0x0002:{  // 0x8XY2, AND
                    uint8_t X = (opcode & 0x0F00) >> 8;
                    uint8_t Y = (opcode & 0x00F0) >> 4; 
                    V[X] =V[X] & V[Y];
                    V[0xF] = 0;
                    break;
                }
                case 0x0003:{// 0x8XY3, XOR
                    uint8_t X = (opcode & 0x0F00) >> 8;
                    uint8_t Y = (opcode & 0x00F0) >> 4; 
                    V[X] =V[X] ^ V[Y];
                    V[0xF] = 0;
                    break;
                }
                case 0x0004:{ // 0x8XY4, ADD w/ Carry
                    uint8_t X = (opcode & 0x0F00) >> 8;
                    uint8_t Y = (opcode & 0x00F0) >> 4; 
                    bool cond = V[X] + V[Y] > 255;
                    V[X] = V[X] + V[Y];
                    if (cond){
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    break;
                }
                case 0x0005:{// 0x8XY5, SUB x - y w/ Carry
                    uint8_t X = (opcode & 0x0F00) >> 8;
                    uint8_t Y = (opcode & 0x00F0) >> 4; 
                    bool cond = V[X] >= V[Y];
                    V[X] = V[X] - V[Y];
                    if (cond){
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    break;
                }
                case 0x0006:{ // 0x8XY6, Shift Right
                    uint8_t X = (opcode & 0x0F00) >> 8;
                    uint8_t Y = (opcode & 0x00F0) >> 4; 
                    uint8_t LSB = (V[Y] & 0x0001);
                    V[X] = V[Y];
                    V[X] = V[X] >> 1;
                    V[0xF] = LSB;
                    break;
                }
                case 0x0007:{// 0x8XY7, SUB y- x w/ Carry
                    uint8_t X = (opcode & 0x0F00) >> 8;
                    uint8_t Y = (opcode & 0x00F0) >> 4; 
                    bool cond = V[Y] >= V[X];
                    V[X] = V[Y] - V[X];
                    if ( cond){
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    break;
                }
                case 0x000E:{ // 0x8XYE, Shift Left
                    uint8_t X = (opcode & 0x0F00) >> 8;
                    uint8_t Y = (opcode & 0x00F0) >> 4; 
                    uint8_t MSB = (V[Y] & 0x80) >> 7;
                    V[X] = V[Y];
                    V[X] = V[X] << 1;
                    V[0xF] = MSB;
                    break;
                }
            }
            break;
        }

        case 0x9000: {// 0x9XY0, Conditional Skip
            uint16_t X = (opcode & 0x0F00) >> 8;
            uint16_t Y = (opcode & 0x00F0) >> 4; 
            if (V[X] != V[Y]){
                pc += 2;
            }
            break;
        }

        case 0xA000: {// 0xANNN, set I  
            uint16_t NNN = opcode & 0x0FFF;
            I = NNN; 
            break;
        }
        case 0xB000: {// 0xBNNN, jump w/ offset 
            uint16_t NNN = opcode & 0x0FFF;
            pc = NNN + V[0];
            break;
        }

        case 0xC000: {// 0xCXNN, jump w/ offset 

            std::random_device rand;
            std::mt19937 gen(rand());
            std::uniform_int_distribution<> distrib(0,255);
            uint16_t X = (opcode & 0x0F00) >> 8;
            uint16_t NN = opcode & 0x00FF; 
            V[X] = distrib(gen) & NN;
            break;
        }

        case 0xD000:{ // 0xDXYN, Display
            uint8_t X = (opcode & 0x0F00) >> 8;
            uint8_t Y = (opcode & 0x00F0) >> 4;
            uint8_t N = (opcode & 0x000F);

            uint16_t startX = V[X] % DISPLAY_W;    // wrap starting position
            uint16_t startY = V[Y] % DISPLAY_H;

            drawFlag = true;
            V[0xF] = 0;
            for (uint16_t i = 0; i < N; i ++){

                uint16_t y = startY + i;
                if (y >= DISPLAY_H) break;

                uint16_t sprite = memory[I + i];

                for (uint16_t j  = 0; j < 8; j++){

                    uint16_t x = startX + j;
                    if (x >= DISPLAY_W) break;
                    
                    uint16_t pixel = (sprite >> (7 - j) & 1);

                    if (pixel){
                        
                        uint16_t index = y * DISPLAY_W + x;
                        bool screen_pixel = display[index];
                        if (screen_pixel) {
                            display[index] = 0;
                            V[0xF] = 1;
                        } else{
                            display[index] = 1;
                        }
                    }

                }

               
            }

            break;
        }

        case 0xE000:{
            switch(opcode & 0x00FF){
                case 0x009E:{ // skip if key pressed
                    uint8_t X = (opcode & 0x0F00) >> 8;
                    if (keys[V[X]] == 1){
                        pc += 2;
                    }
                    break;
                }
                case 0x00A1:{ // skip if key not pressed
                    uint8_t X = (opcode & 0x0F00) >> 8;
                    if (keys[V[X]] != 1){
                        pc += 2;
                    }
                    break;
                }
            }
            break;
        }

        case 0xF000:{
            switch(opcode & 0x00FF){
                case 0x0007:{ // set vx to delay
                    uint8_t X = (opcode & 0x0F00) >> 8;
                    V[X] = delayTimer;
                    break;
                }
                case 0x0015:{ // set delay to vx
                    uint8_t X = (opcode & 0x0F00) >> 8;
                    delayTimer = V[X];
                    break;
                }
                case 0x0018:{ //set sound to vx
                    uint8_t X = (opcode & 0x0F00) >> 8;
                    soundTimer = V[X];
                    break;
                }
                case 0x001E:{ //i + vx
                    uint8_t X = (opcode & 0x0F00) >> 8;
                    I += V[X];
                    break;
                }
                case 0x000A:{ //get key
                    uint8_t X = (opcode & 0x0F00) >> 8;
                    int i = 0;
                    for ( i = 0; i < 16; i++){
                        if (keys[i] == 1){
                        
                            V[X] = i;
                            break;
                        }
                    }
                    if (i == 16){
                        pc -= 2;
                    }
                    break;
                }

                case 0x0029:{ //i = address of font in vx
                    uint8_t X = (opcode & 0x0F00) >> 8;
                    I = FONTSET_ADDR + V[X] * 5;
                    break;
                }
                case 0x0033:{ //bcd
                    uint8_t X = (opcode & 0x0F00) >> 8;
                    memory[I] = V[X]/100;
                    memory[I + 1] = (V[X]/10) % 10;
                    memory[I+ 2] = (V[X] % 100) % 10;
                    break;
                }

                case 0x0055:{ //store
                    uint8_t X = (opcode & 0x0F00) >> 8;
                    for(int i = 0; i <= X; i++){
                        memory[I + i] =  V[ i];
                    }
                    I += X + 1;
                    break;
                }

                case 0x0065:{ //load
                    uint8_t X = (opcode & 0x0F00) >> 8;
                    for(int i = 0; i <= X; i++){
                        V[ i] = memory[I + i];
                    }
                    I += X + 1;
                    break;
                }
                
            }
            break;
        }

        default: {
            //std::cerr << std::hex << std::uppercase << opcode << "\n";
            break;
        }
    }
}

void Chip8::tickTimers(){
    if (delayTimer > 0){
        delayTimer --;
    }
    if (soundTimer > 0){
        soundTimer --;
    }
}