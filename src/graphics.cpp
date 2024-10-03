#include "graphics.h"
#include "colors.h"
#include <GL/freeglut_std.h>
#include <GL/gl.h>
#include <GL/glut.h>

namespace graphics {

void init(void (*callback)(int), void (*display)(),
          void (*key_press)(unsigned char, int, int),
          void (*key_release)(unsigned char, int, int), int argc, char **argv) {
    glutInit(&argc, argv);

    // Set display mode (single buffer and RGB color)
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);

    // Set window size
    glutInitWindowSize(1000, 500);

    // Create a window
    glutCreateWindow("CHIP8 Emulator");
    glClearColor(Colors::BG.r, Colors::BG.g, Colors::BG.b, 1.0);

    // Set the display callback
    glutDisplayFunc(display);

    // Set keyboard callbacks
    glutKeyboardFunc(key_press);
    glutKeyboardUpFunc(key_release);

    glutTimerFunc(0, callback, 0);

    // Enter the GLUT main loop
    glutMainLoop();
}

void draw_square(int const x, int const y) {
    // Draw a square using GL_QUADS
    double const x_orig{2.0f / DISPLAY_WIDTH};
    double const y_orig{2.0f / DISPLAY_HEIGHT};

    double const x_offset(2.0f * x / DISPLAY_WIDTH);
    double const y_offset(2.0f * y / DISPLAY_HEIGHT);

    glBegin(GL_QUADS);
    glVertex2f(-1.0f + x_offset, -1.0f + y_offset); // Bottom-left corner
    glVertex2f(-1.0f + x_offset + x_orig,
               -1.0f + y_offset); // Bottom-right corner
    glVertex2f(-1.0f + x_offset + x_orig,
               -1.0f + y_offset + y_orig);                   // Top-right corner
    glVertex2f(-1.0f + x_offset, -1.0f + y_offset + y_orig); // Top-left corner
    glEnd();
}

}; // namespace graphics
