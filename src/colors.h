#include <GL/gl.h>

struct Color {
    GLfloat r;
    GLfloat g;
    GLfloat b;
};

struct Colors {
    static constexpr Color BG {90/255.0, 82/255.0, 142/255.0};
    static constexpr Color FG {225/255.0, 175/255.0, 209/255.0};
};

