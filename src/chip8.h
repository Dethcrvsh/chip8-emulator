#include <cstddef>
#include <cstdint>
#include <stack>
#include <string>
#include <unordered_map>

#pragma once

struct OPCodeTester;


class CHIP8 {
public:
    static int const DISPLAY_WIDTH{64};
    static int const DISPLAY_HEIGHT{32};
    static int const SPRITE_WIDTH{8};
    static int constexpr REFRESH_RATE {500};
    static std::unordered_map<char, int> const KEYMAP;

    // Legacy sets PC to NNN + V0, otherwise NNN + VX
    bool USE_LEGACY_JUMP{true};
    // Legacy first sets VX = VY
    bool USE_LEGACY_SHIFT{true};
    // Legacy does not affect the VF flag
    bool USE_LEGACY_INDEX_ADD{false};
    // Legacy increments the I register
    bool USE_LEGACY_LOAD_STORE{true};

    friend struct OPCodeTester;

    bool display[DISPLAY_HEIGHT][DISPLAY_WIDTH];
    bool display_buffer[DISPLAY_HEIGHT][DISPLAY_WIDTH];
    // Do NOT touch this or the race will condition you
    uint16_t keystates {};

    CHIP8();
    ~CHIP8() = default;
    CHIP8(const CHIP8 &other) noexcept = default;
    CHIP8(CHIP8& other) noexcept = default;
    CHIP8& operator=(CHIP8& other) noexcept = default;
    CHIP8(CHIP8&& other) noexcept = default;
    CHIP8& operator=(CHIP8&& other) noexcept = default;

    void run_rom(std::string const& path);
    void cycle(bool const force = false);
    uint16_t fetch();

    void pause();
    void resume();

private:
    uint8_t memory[4096];
    uint16_t pc {};
    uint16_t I {};
    std::stack<uint16_t> stack {};
    uint8_t delay_timer {60};
    uint8_t sound_timer {};
    uint8_t registers[16];

    double accum_time {};
    bool is_paused {false};

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

