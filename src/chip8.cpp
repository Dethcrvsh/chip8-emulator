#include "chip8.h"
#include "graphics.h"
#include <GL/freeglut_std.h>
#include <GL/gl.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <limits>
#include <random>

CHIP8::CHIP8() {
    // Initialize the font
    std::byte font[5 * 16]{
        to_byte(0xF0), to_byte(0x90), to_byte(0x90),
        to_byte(0x90), to_byte(0xF0), // 0
        to_byte(0x20), to_byte(0x60), to_byte(0x20),
        to_byte(0x20), to_byte(0x70), // 1
        to_byte(0xF0), to_byte(0x10), to_byte(0xF0),
        to_byte(0x80), to_byte(0xF0), // 2
        to_byte(0xF0), to_byte(0x10), to_byte(0xF0),
        to_byte(0x10), to_byte(0xF0), // 3
        to_byte(0x90), to_byte(0x90), to_byte(0xF0),
        to_byte(0x10), to_byte(0x10), // 4
        to_byte(0xF0), to_byte(0x80), to_byte(0xF0),
        to_byte(0x10), to_byte(0xF0), // 5
        to_byte(0xF0), to_byte(0x80), to_byte(0xF0),
        to_byte(0x90), to_byte(0xF0), // 6
        to_byte(0xF0), to_byte(0x10), to_byte(0x20),
        to_byte(0x40), to_byte(0x40), // 7
        to_byte(0xF0), to_byte(0x90), to_byte(0xF0),
        to_byte(0x90), to_byte(0xF0), // 8
        to_byte(0xF0), to_byte(0x90), to_byte(0xF0),
        to_byte(0x10), to_byte(0xF0), // 9
        to_byte(0xF0), to_byte(0x90), to_byte(0xF0),
        to_byte(0x90), to_byte(0x90), // A
        to_byte(0xE0), to_byte(0x90), to_byte(0xE0),
        to_byte(0x90), to_byte(0xE0), // B
        to_byte(0xF0), to_byte(0x80), to_byte(0x80),
        to_byte(0x80), to_byte(0xF0), // C
        to_byte(0xE0), to_byte(0x90), to_byte(0x90),
        to_byte(0x90), to_byte(0xE0), // D
        to_byte(0xF0), to_byte(0x80), to_byte(0xF0),
        to_byte(0x80), to_byte(0xF0), // E
        to_byte(0xF0), to_byte(0x80), to_byte(0xF0),
        to_byte(0x80), to_byte(0x80) // F
    };
    std::copy(std::begin(font), std::end(font), &(memory[0x50]));
    run_rom("roms/ibm-logo.ch8");
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

void CHIP8::cycle() {
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
                    glClear(GL_COLOR_BUFFER_BIT);
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


        // NOTE: Ambiguous
        // TODO: Figure it out
        // Jump with offset
        case 0xB000:
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
                    break;

                // Binary AND
                case 0x0002:
                    registers[X] = registers[X] & registers[Y];
                    break;

                // Logical XOR
                case 0x0003:
                    registers[X] = registers[X] ^ registers[Y];
                    break;

                // Add
                case 0x0004:
                    registers[X] = registers[X] + registers[Y];
                    // If the sum is smaller than the operand, overflow occured
                    registers[0xF] = registers[X] < registers[Y];
                    break;

                // Subtract X-Y
                case 0x0005:
                    // If X is bigger than Y, underflow occurs
                    registers[0xF] = registers[X] > registers[Y];
                    registers[X] = registers[X] - registers[Y];
                    break;

                // Subtract Y-X
                case 0x0007:
                    // If Y is bigger than X, underflow occurs
                    registers[0xF] = registers[Y] > registers[X];
                    registers[X] = registers[Y] - registers[X];
                    break;

                // NOTE: Ambiguous
                // Shift Right
                case 0x0006:
                    if (true /* Put something here */) {
                        registers[X] = registers[Y];
                    }
                    registers[0xF] = registers[X] & 0x0001;
                    registers[X] = registers[X] >> 1;
                    break;

                // NOTE: Ambiguous
                // Shift Right
                case 0x000E:
                    if (true /* Put something here */) {
                        registers[X] = registers[Y];
                    }
                    registers[0xF] = (registers[X] & 0x8000) >> 15;
                    registers[X] = registers[X] << 1;
                    break;
            }

            break;
        }
    }
}

uint16_t CHIP8::fetch() {
    pc += 2;
    return (static_cast<uint16_t>(static_cast<uint8_t>(memory[pc - 2]) << 8) |
    static_cast<uint16_t>(static_cast<uint8_t>(memory[pc - 1])));
}

std::byte CHIP8::to_byte(int const value) {
    return static_cast<std::byte>(value);
}

void CHIP8::draw(uint16_t const X, uint16_t const Y, uint16_t const N) {
    uint8_t const x_reg{static_cast<uint8_t>(registers[X] % DISPLAY_WIDTH)};
    uint8_t const y_reg{static_cast<uint8_t>(registers[Y] % DISPLAY_HEIGHT)};
    registers[0xF] = false;

    for (int i{0}; i < N; i++) {
        std::byte pixels{memory[I + i]};

        for (int j{0}; j < 8; j++) {
            bool *pixel{&display[y_reg - i + 16][x_reg + 8 - j]};
            // eww
            uint8_t const new_pixel{static_cast<uint8_t>(static_cast<uint8_t>(pixels) & 0x01)};

            // TODO: what is happening here
            
            // XOR the current pixel with the last bit of the sprite
            registers[0xF] |= *pixel && new_pixel;
            *pixel = *pixel != new_pixel;

            registers[0xF] = *pixel && new_pixel;

            pixels = pixels >> 1;
        }
    }
}

void CHIP8::timer_tick(int const t) {
    delay_timer = std::max(0, delay_timer - t);
    sound_timer = std::max(0, sound_timer - t);
};
