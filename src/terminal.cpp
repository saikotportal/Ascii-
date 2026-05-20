#include "../include/color.h"
#include <cstdio>
#include <termios.h>
#include <unistd.h>

static struct termios originalTermios;

void Term::enableRaw() {
    tcgetattr(STDIN_FILENO, &originalTermios);
    struct termios raw = originalTermios;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN]  = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void Term::disableRaw() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &originalTermios);
}

char Term::getChar() {
    char c = 0;
    read(STDIN_FILENO, &c, 1);
    return c;
}
