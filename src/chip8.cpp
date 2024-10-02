#include "chip8.h"
#include <GL/freeglut_std.h>
#include <GL/gl.h>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <random>
#include <cstring>


std::unordered_map<char, int> const CHIP8::KEYMAP {
    {'1', 0x1},
    {'2', 0x2},
    {'3', 0x3},
    {'4', 0xC},
    {'q', 0x4},
    {'Q', 0x4},
    {'w', 0x5},
    {'W', 0x5},
    {'e', 0x6},
    {'E', 0x6},
    {'r', 0xD},
    {'R', 0xD},
    {'a', 0x7},
    {'A', 0x7},
    {'s', 0x8},
    {'S', 0x8},
    {'d', 0x9},
    {'D', 0x9},
    {'f', 0xE},
    {'F', 0xE},
    {'z', 0xA},
    {'Z', 0xA},
    {'x', 0x0},
    {'X', 0x0},
    {'c', 0xB},
    {'C', 0xB},
    {'v', 0xF},
    {'V', 0xF},
};

CHIP8::CHIP8() {
    // Initialize the font
    uint8_t font[5 * 16]{
        0xF0, 0x90, 0x90,
        0x90, 0xF0, // 0
        0x20, 0x60, 0x20,
        0x20, 0x70, // 1
        0xF0, 0x10, 0xF0,
        0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0,
        0x10, 0xF0, // 3
        0x90, 0x90, 0xF0,
        0x10, 0x10, // 4
        0xF0, 0x80, 0xF0,
        0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0,
        0x90, 0xF0, // 6
        0xF0, 0x10, 0x20,
        0x40, 0x40, // 7
        0xF0, 0x90, 0xF0,
        0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0,
        0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0,
        0x90, 0x90, // A
        0xE0, 0x90, 0xE0,
        0x90, 0xE0, // B
        0xF0, 0x80, 0x80,
        0x80, 0xF0, // C
        0xE0, 0x90, 0x90,
        0x90, 0xE0, // D
        0xF0, 0x80, 0xF0,
        0x80, 0xF0, // E
        0xF0, 0x80, 0xF0,
        0x80, 0x80 // F
    };
    std::copy(std::begin(font), std::end(font), &(memory[0x50]));
    run_rom("roms/6-keypad.ch8");
}

void CHIP8::run_rom(std::string const path) {
    std::ifstream ifs(path, std::ios::binary);

    if (!ifs.is_open()) {
        std::cerr << "Could not open file" << std::endl;
        return;
    }

    // Start reading into RAM at adress 0x200
    ifs.read(reinterpret_cast<char *>(&memory[0x200]), 4096);
    // Start the program
    pc = 0x200;
}

void CHIP8::cycle(bool const force) {
    if (is_paused && !force) {
        return;
    }

    // Tick down the timers at 60Hz
    accum_time += 60.0 / REFRESH_RATE;
    double tick;
    modf(accum_time, &tick);
    timer_tick(tick);
    accum_time -= tick;


    uint16_t const op{fetch()};

    uint16_t const X{static_cast<uint16_t>((op & OpMask::X) >> 8)};
    uint16_t const Y{static_cast<uint16_t>((op & OpMask::Y) >> 4)};
    uint16_t const N{static_cast<uint16_t>(op & OpMask::N)};
    uint16_t const NN{static_cast<uint16_t>(op & OpMask::NN)};
    uint16_t const NNN{static_cast<uint16_t>(op & OpMask::NNN)};

    switch (op & 0xF000) {
        case 0x0000:
            switch (op) {
                // Clear the screen
                case 0x00E0:
                    std::fill(
                        &display[0][0],
                        &display[0][0] + DISPLAY_WIDTH * DISPLAY_HEIGHT,
                        false 
                    );
                    break;

                // Return from subroutine
                case 0x00EE:
                    pc = stack.top();
                    stack.pop();
                    break;
            }
            break;

        // Jump
        case 0x1000:
            pc = NNN;
            break;


        case 0xB000:
            if (USE_LEGACY_JUMP) {
                pc = registers[0x0] + NNN;
            } else {
                pc = registers[X] + NNN;
            }
            break;

        // Call Subroutine
        case 0x2000:
            stack.push(pc);
            pc = NNN;
            break;

        // Skip if Equal
        case 0x3000:
            pc += 2 * (registers[X] == NN);
            break;

        // Skip if not Equal
        case 0x4000:
            pc += 2 * (registers[X] != NN);
            break;

        // Skip if registers are Equal
        case 0x5000:
            pc += 2 * (registers[X] == registers[Y]);
            break;

        // Skip if registers are not Equal
        case 0x9000:
            pc += 2 * (registers[X] != registers[Y]);
            break;

        //  Set
        case 0x6000:
            registers[X] = NN;
            break;

        // Add
        case 0x7000:
            registers[X] += NN;
            break;

        // Set Index Register
        case 0xA000:
            I = NNN;
            break;

        // Draw
        case 0xD000:
            draw(X, Y, N);
            break;

        // Random
        case 0xC000: {
            std::random_device rng{};
            std::mt19937 gen{rng()};
            std::uniform_int_distribution<uint16_t> dis{0, std::numeric_limits<uint16_t>::max()};

            uint16_t const rand{dis(gen)};
            registers[X] = rand & NN;

            break;
        }

        case 0x8000: {
            uint16_t const instr{static_cast<uint16_t>(op & 0x000F)};

            switch(instr) {
                // Set
                case 0x0000:
                    registers[X] = registers[Y];
                    break;

                // Binary OR
                case 0x0001:
                    registers[X] = registers[X] | registers[Y];
                    registers[0xF] = 0;
                    break;

                // Binary AND
                case 0x0002:
                    registers[X] = registers[X] & registers[Y];
                    registers[0xF] = 0;
                    break;

                // Logical XOR
                case 0x0003:
                    registers[X] = registers[X] ^ registers[Y];
                    registers[0xF] = 0;
                    break;

                // Add
                case 0x0004:
                    registers[X] = registers[X] + registers[Y];
                    // If the sum is smaller than the operand, overflow occured
                    registers[0xF] = registers[X] < registers[Y];
                    break;

                // Subtract X-Y
                case 0x0005: {
                    bool const carry {registers[Y] > registers[X]};
                    registers[X] = registers[X] - registers[Y];
                    registers[0xF] = !carry;
                    break;
                }

                // Subtract Y-X
                case 0x0007: {
                    bool const carry {registers[X] > registers[Y]};
                    registers[X] = registers[Y] - registers[X];
                    registers[0xF] = !carry;
                    break;
                }

                // Shift Right
                case 0x0006: {
                    if (USE_LEGACY_SHIFT) {
                        registers[X] = registers[Y];
                    }
                    // fuck brace initialization, I know this cast is fine
                    // I don't need to tell you that im not stupid, compiler
                    uint8_t const carry = registers[X] & 0x01;
                    registers[X] >>= 0x1;
                    registers[0xF] = carry;
                    break;
                }

                // Shift Left
                case 0x000E:
                    if (USE_LEGACY_SHIFT) {
                        registers[X] = registers[Y];
                    }
                    // looking at you g++
                    uint8_t const carry = (registers[X] & 0x80) >> 7;
                    registers[X] <<= 0x1; 
                    registers[0xF] = carry;
                    break;
            }
            break;
        }

        // Key presses
        case 0XE000:
            switch(NN) {
                // Skip if key in X is pressed
                case 0x009E:
                    pc += 2 * ((keystates & (0x1 << registers[X])) != 0);
                    break;

                // Skip if key in X is not pressed
                case 0x00A1:
                    pc += 2 * ((keystates & (0x1 << registers[X])) == 0);
                    break;
            }
            break;

        // Timers
        case 0xF000:
            switch(NN) {
                // Set X to Delay Timer
                case 0x0007:
                    registers[X] = delay_timer;
                    break;

                // Set the Delay Timer to X
                case 0x0015:
                    delay_timer = registers[X];
                    break;

                // Set the Sound Timer to X
                case 0x0018:
                    sound_timer = registers[X];
                    break;

                // Add X to Index
                case 0x001E:
                    I += registers[X];

                    if (USE_LEGACY_INDEX_ADD)
                        // Set overflow flag
                        registers[0xF] = I < registers[X];
                    break;

                // Get key (block until a key is pressed)
                case 0x000A:
                    if (keystates) {
                        // Set X to the first key pressed that is found
                        for (int i {}; i < 16; i++) {
                            if (keystates & (0x1 << i)) {
                                registers[X] = i;
                                break;
                            }
                        }

                    } else {
                        pc -= 2;
                    }
                    break;

                // Font character
                case 0x0029:
                    I = 0x0050 + 5 * registers[X]; 
                    break;
                    
                // Binary coded decimal conversion
                case 0x0033: {
                    uint8_t const num {registers[X]};
                    memory[I] = num / 100;
                    memory[I+1] = (num % 100) / 10;
                    memory[I+2] = num % 10;
                    break;
                }

                // Store memory
                case 0x0055:
                    std::memcpy(memory + I, registers, X + 1);

                    if (USE_LEGACY_LOAD_STORE) {
                        I += X + 1;
                    }
                    break;

                // Load memory
                case 0x0065:
                    std::memcpy(registers, memory + I, X + 1);

                    if (USE_LEGACY_LOAD_STORE) {
                        I += X + 1;
                    }
                    break;
            }
    }
}

uint16_t CHIP8::fetch() {
    pc += 2;
    return (static_cast<uint16_t>(memory[pc - 2] << 8) |
    static_cast<uint16_t>(memory[pc - 1]));
}

void CHIP8::pause() {
    is_paused = true;
}

void CHIP8::resume() {
    is_paused = false;
}

void CHIP8::draw(uint16_t const X, uint16_t const Y, uint16_t const N) {
    uint8_t const x_reg{static_cast<uint8_t>(registers[X] % DISPLAY_WIDTH)};
    uint8_t const y_reg{static_cast<uint8_t>(registers[Y] % DISPLAY_HEIGHT)};
    registers[0xF] = false;

    for (int i{0}; i < N; i++) {
        uint8_t pixels{memory[I+i]};

        for (int j{0}; j < 8; j++) {
            int const pixel_x {x_reg + j};
            int const pixel_y {y_reg + i};

            // Skip if drawing outside the display
            if (pixel_x >= DISPLAY_WIDTH || pixel_y >= DISPLAY_HEIGHT) {
                continue;
            }

            bool &pixel = display[pixel_y][pixel_x];
            // Extract the corresponding pixel
            bool new_pixel = (pixels >> (7-j)) & 0x01;

            registers[0xF] |= pixel && new_pixel;
            pixel ^= new_pixel;
        }
    }
}

void CHIP8::timer_tick(int const t) {
    delay_timer = std::max(0, delay_timer - t);
    sound_timer = std::max(0, sound_timer - t);
};

