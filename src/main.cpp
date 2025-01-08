#include "chip8.h"
#include "graphics.h"
#include "opcode_tester.h"
#include "colors.h"
#include <GL/freeglut_std.h>
#include <chrono>
#include <cmath>
#include <cstring>
#include <iostream>

CHIP8 chip8{};;

void on_press(unsigned const char key, int, int) {
    if (chip8.KEYMAP.find(key) != chip8.KEYMAP.end()) {
        uint16_t const current_key = chip8.KEYMAP.at(key);
        // Set the corresponding bit for the key
        chip8.keystates |= (0x001) << current_key;
    }
}

void on_release(unsigned const char key, int, int) {
    if (chip8.KEYMAP.find(key) != chip8.KEYMAP.end()) {
        uint16_t const current_key = chip8.KEYMAP.at(key);
        // Set the corresponding bit for the key
        chip8.keystates &= ~((0x001) << current_key);
    }
}

void loop() {
    static auto last_frame_time = std::chrono::high_resolution_clock::now();
    static double accum {};

    auto now = std::chrono::high_resolution_clock::now();
    double const elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - last_frame_time).count();
    last_frame_time = now;

    double const target_frame_time {1000000.0 / CHIP8::REFRESH_RATE};

    accum += elapsed;

    while (accum >= target_frame_time) {
        accum -= target_frame_time;
        chip8.cycle();
        if (chip8.do_redraw) {
            glutPostRedisplay();
        }
    }

    glutPostRedisplay();
}

void draw() {
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(Colors::FG.r, Colors::FG.g, Colors::FG.b);

    for (int y = 0; y < chip8.DISPLAY_HEIGHT; y++) {
        for (int x = 0; x < chip8.DISPLAY_WIDTH; x++) {
            if (chip8.display[y][x]) {
                graphics::draw_square(x, chip8.DISPLAY_HEIGHT - y - 1);
            }
        }
    }

    glutSwapBuffers();
    glFlush();
}

int main(int argc, char **argv) {
    if (argc <= 1) {
        chip8.run_rom("");
        graphics::init(loop, draw, on_press, on_release, argc, argv);
    }

    // Run the test module
    if (std::string(argv[1]) == "--opcode-test") {
        OPCodeTester tester {};
        tester.run(chip8);
    } else {
        chip8.run_rom(argv[1]);
        graphics::init(loop, draw, on_press, on_release, argc, argv);
    }

    return 0;
}

