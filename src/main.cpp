#include "chip8.h"
#include "graphics.h"
#include "opcode_tester.h"
#include "colors.h"
#include <GL/freeglut_std.h>
#include <chrono>
#include <cmath>
#include <cstring>
#include <algorithm>

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

void loop(int) {
    auto const start{std::chrono::high_resolution_clock::now()};

    chip8.cycle();

    glutPostRedisplay();

    auto const end{std::chrono::high_resolution_clock::now()};

    double const elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
        .count();

    glutTimerFunc(std::max((1000.0-elapsed) / CHIP8::REFRESH_RATE, 0.0), loop, 0);
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
        return 0;
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

