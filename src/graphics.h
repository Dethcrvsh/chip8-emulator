#include <GL/freeglut_std.h>
#include <GL/gl.h>
#include <GL/glut.h>

#pragma once


namespace graphics {

int const DISPLAY_WIDTH{64};
int const DISPLAY_HEIGHT{32};

void init(void (*callback)(int), void (*display)(),
          void (*key_press)(unsigned char, int, int),
          void (*key_release)(unsigned char, int, int), int argc, char **argv);
void display();
void draw_square(int const x, int const y);

}; // namespace graphics
