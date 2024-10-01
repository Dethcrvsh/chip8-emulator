#include "chip8.h"
#include "graphics.h"
#include "opcode_tester.h"
#include <GL/freeglut_std.h>
#include <chrono>
#include <cmath>
#include <cstring>

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

    glutTimerFunc((1000.0 - elapsed) / CHIP8::REFRESH_RATE, loop, 0);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

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
    // Run the test module
    if (argc > 1 && std::string(argv[1]) == "--opcode-test") {
        OPCodeTester tester {};
        tester.run(chip8);
    } else {
        graphics::init(loop, display, on_press, on_release, argc, argv);
    }

    return 0;
}

