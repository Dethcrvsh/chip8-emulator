#include "chip8.h"
#include <GL/freeglut_std.h>
#include <GL/gl.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include "graphics.h"
#include <iostream>
#include <fstream>

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
    ifs.read(reinterpret_cast<char*>(&memory[0x200]), 4096);
    // Start the program
    pc = 0x200;
}

void CHIP8::cycle() {
    // Tick down the timers at 60Hz
    accum_time += 60.0/REFRESH_RATE;
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
            }
            break;

        // Jump
        case 0x1000:
            pc = NNN;
            break;

        //  Set
        case 0x6000:
            registers[X] = NN;
            break;

        // Add
        case 0x7000:
            registers[X] += NN;
            break;

        // Set index register
        case 0xA000:
            I = NNN;
            break;

        // Draw
        case 0xD000:
            draw(X, Y, N);
    }
}

uint16_t CHIP8::fetch() {
    pc += 2;
    return (
        static_cast<uint16_t>(static_cast<uint8_t>(memory[pc - 2]) << 8) |
        static_cast<uint16_t>(static_cast<uint8_t>(memory[pc - 1]))
    );
}

std::byte CHIP8::to_byte(int const value) {
    return static_cast<std::byte>(value);
}

void CHIP8::draw(uint16_t const X, uint16_t const Y, uint16_t const N) {
    uint8_t const x_reg{static_cast<uint8_t>(registers[X] % DISPLAY_WIDTH)};
    uint8_t const y_reg{static_cast<uint8_t>(registers[Y] % DISPLAY_HEIGHT)};

    for (int i{0}; i < N; i++) {
        std::byte pixels{memory[I+i]};

        for (int j{0}; j < 8; j++) {
            bool *pixel{&display[y_reg-i+16][x_reg+8-j]};

            // XOR the current pixel with the last bit of the sprite
            registers[0xF] |= *pixel && (static_cast<uint8_t>(pixels) & 0x01);
            *pixel = *pixel != static_cast<bool>(static_cast<uint8_t>(pixels) & 0x01);

            pixels = pixels >> 1;
        }
    }
}

void CHIP8::timer_tick(int const t) {
    delay_timer = std::max(0, delay_timer-t);
    sound_timer = std::max(0, sound_timer-t);
};
