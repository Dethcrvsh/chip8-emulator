#include <cstddef>
#include <cstdint>
#include <stack>
#include <string>
#include <unordered_map>


class CHIP8 {
public:
    static int const DISPLAY_WIDTH{64};
    static int const DISPLAY_HEIGHT{32};
    static int const SPRITE_WIDTH{8};
    static int constexpr REFRESH_RATE {700};

    std::unordered_map<char, int> const keymap {
        {'1', 0x1},
        {'2', 0x2},
        {'3', 0x3},
        {'4', 0xC},
        {'Q', 0x4},
        {'W', 0x5},
        {'E', 0x6},
        {'R', 0xD},
        {'A', 0x7},
        {'S', 0x8},
        {'D', 0x9},
        {'F', 0xE},
        {'Z', 0xA},
        {'X', 0x0},
        {'C', 0xB},
        {'V', 0xF},
    };

    bool display[DISPLAY_HEIGHT][DISPLAY_WIDTH];
    // Do NOT touch this or the race will condition you
    bool keystates[16] {};

    CHIP8();
    void run_rom(std::string const path);
    void cycle();
    uint16_t fetch();

private:
    std::byte memory[4096];
    uint16_t pc {};
    uint16_t I {};
    std::stack<uint16_t> stack {};
    uint8_t delay_timer {static_cast<uint8_t>(127)};
    uint8_t sound_timer {};
    uint8_t registers[16];
    double accum_time {};

    enum OpMask : uint16_t {
        W = 0xF000,
        X = 0x0F00,
        Y = 0x00F0,
        N = 0x000F,
        NN = 0x00FF,
        NNN = 0x0FFF,
    };

    std::byte to_byte(int const value);
    void draw(uint16_t const X, uint16_t const Y, uint16_t const N);
    void timer_tick(int);
};
