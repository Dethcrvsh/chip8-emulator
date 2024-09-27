#include "chip8.h"
#include "graphics.h"
#include <GL/freeglut_std.h>
#include <chrono>
#include <cmath>
#include <cstring>
#include <iostream>

CHIP8 chip8{};

void on_press(unsigned const char key, int, int) {
    if (chip8.keymap.find(key) != chip8.keymap.end()) {
        chip8.keystates[chip8.keymap.at(key)] = true;
    }
}

void on_release(unsigned const char key, int, int) {
    if (chip8.keymap.find(key) != chip8.keymap.end()) {
        chip8.keystates[chip8.keymap.at(key)] = false;
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

    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            if (chip8.display[y][x]) {
                graphics::draw_square(x, y);
            }
        }
    }

    glutSwapBuffers();
    glFlush();
}

int main(int argc, char **argv) {
    graphics::init(loop, display, on_press, on_release, argc, argv);

    return 0;
}
