#pragma once
#include <string>
#include <sstream>

namespace Color {
    // Foreground colors
    const std::string RESET   = "\033[0m";
    const std::string BOLD    = "\033[1m";
    const std::string DIM     = "\033[2m";

    const std::string BLACK   = "\033[30m";
    const std::string RED     = "\033[31m";
    const std::string GREEN   = "\033[32m";
    const std::string YELLOW  = "\033[33m";
    const std::string BLUE    = "\033[34m";
    const std::string MAGENTA = "\033[35m";
    const std::string CYAN    = "\033[36m";
    const std::string WHITE   = "\033[37m";

    // Bright variants
    const std::string BRED    = "\033[91m";
    const std::string BGREEN  = "\033[92m";
    const std::string BYELLOW = "\033[93m";
    const std::string BBLUE   = "\033[94m";
    const std::string BMAGENTA= "\033[95m";
    const std::string BCYAN   = "\033[96m";
    const std::string BWHITE  = "\033[97m";

    // Backgrounds
    const std::string BG_BLACK  = "\033[40m";
    const std::string BG_RED    = "\033[41m";
    const std::string BG_GREEN  = "\033[42m";
    const std::string BG_BLUE   = "\033[44m";
    const std::string BG_CYAN   = "\033[46m";

    inline std::string paint(const std::string& code, const std::string& text) {
        return code + text + RESET;
    }
    inline std::string bold(const std::string& text) {
        return BOLD + text + RESET;
    }
    inline std::string red(const std::string& t)    { return paint(RED, t); }
    inline std::string green(const std::string& t)  { return paint(GREEN, t); }
    inline std::string yellow(const std::string& t) { return paint(YELLOW, t); }
    inline std::string cyan(const std::string& t)   { return paint(CYAN, t); }
    inline std::string magenta(const std::string& t){ return paint(MAGENTA, t); }
    inline std::string white(const std::string& t)  { return paint(WHITE, t); }
    inline std::string bred(const std::string& t)   { return paint(BRED, t); }
    inline std::string bgreen(const std::string& t) { return paint(BGREEN, t); }
}

namespace Term {
    inline void clear()    { std::printf("\033[2J\033[H"); }
    inline void hideCursor(){ std::printf("\033[?25l"); }
    inline void showCursor(){ std::printf("\033[?25h"); }
    inline void moveTo(int row, int col) {
        std::printf("\033[%d;%dH", row, col);
    }
    // Raw mode helpers (POSIX)
    void enableRaw();
    void disableRaw();
    char getChar();       // blocking single-char read
}
