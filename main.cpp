#include "include/game.h"
#include <iostream>
#include <csignal>

// Restore terminal on Ctrl+C
static void sigHandler(int) {
    Term::showCursor();
    Term::disableRaw();
    std::printf("\n\033[0mBye!\n");
    std::exit(0);
}

int main() {
    std::signal(SIGINT, sigHandler);
    std::signal(SIGTERM, sigHandler);

    try {
        Game game;
        game.run();
    } catch (const std::exception& e) {
        Term::showCursor();
        Term::disableRaw();
        std::fprintf(stderr, "Fatal error: %s\n", e.what());
        return 1;
    }
    return 0;
}
