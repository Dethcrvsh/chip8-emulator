#include <iomanip>
#include <iostream>
#include "chip8.h"

#pragma once

// Macro to simplify usage
#define ASSERT(condition) op_assert((condition), #condition)

#define SET(array, index, value) \
    do { \
        (array)[(index)] = static_cast<uint8_t>(((value) >> 8) & 0xFF); \
        (array)[(index) + 1] = static_cast<uint8_t>((value) & 0xFF); \
    } while (0)

#define SETUP(op) \
    bool res {true}; \
    opcode((op)); \

#define END(res, os) \
    do { \
        status((res)); \
        os << std::endl; \
    } while (0)


struct OPCodeTester {
    struct ANSI {
        static inline std::string const BOLD {"\033[1m"};
        static inline std::string const RED {"\033[31m"};
        static inline std::string const GREEN {"\033[32m"};
        static inline std::string const YELLOW {"\033[33m"};
        static inline std::string const BLUE {"\033[34m"};
        static inline std::string const MAGENTA {"\033[35m"};
    };

    static inline std::string const OK {ANSI::GREEN + "OK"};
    static inline std::string const ERROR {ANSI::RED + "ERROR"};

    std::ostream &os {std::cout};
    bool error_flag {false};

    bool op_assert(bool const cond, const char *assertion) { 
         if (!cond) {
            if (!error_flag) {
                os << ERROR << "\n";
            }
            os << ANSI::YELLOW << "  : " + std::string(assertion) + "\n";
            error_flag = true;
        }
        
        return cond;
    }

    void status(bool const stat) const {
        if (stat) {
            os << OK;
        }
    }

    void run(CHIP8 &chip8) {
        // Just set everything to bold since it looks nice
        os << ANSI::BOLD;
        os << ANSI::MAGENTA << "----------------------- TESTING OPCODES -----------------------\n" << std::endl;

        // Prevent the chip from automatically running cycles
        chip8.pause();

        /* 
            * ------- TESTS -------
        */

        {
            SETUP("Clear Screen (0x00E0)");

            std::fill(
                &chip8.display[0][0],
                &chip8.display[0][0] + CHIP8::DISPLAY_WIDTH * CHIP8::DISPLAY_HEIGHT,
                true
            );
            SET(chip8.memory, 0x200, 0x00E0);
            chip8.pc = 0x200;
            chip8.cycle(true);

            for (int y {0}; y < CHIP8::DISPLAY_HEIGHT; y++) {
                for (int x {0}; x < CHIP8::DISPLAY_WIDTH; x++) {
                    res &= ASSERT(chip8.display[y][x] == false);
                }
            }
                
            END(res, os);
        }

        {
            SETUP("Jump (0x1NNN)");

            SET(chip8.memory, 0x200, 0x10FF);
            chip8.pc = 0x200;
            chip8.cycle(true);

            res &= ASSERT(chip8.pc == 0x00FF);

            END(res, os);
        }

        {
            SETUP("Jump with Offset(0xB0NN)");

            chip8.registers[0x0] = 2;
            SET(chip8.memory, 0x200, 0xB400);
            chip8.pc = 0x200;
            chip8.cycle(true);

            res &= ASSERT(chip8.pc == 0x0402);

            END(res, os);
        }

        {
            SETUP("Call Subroutine (0x2NNN)");

            SET(chip8.memory, 0x200, 0x20FF);
            chip8.pc = 0x200;
            chip8.cycle(true);

            res &= ASSERT(chip8.pc == 0x00FF);
            res &= ASSERT(chip8.stack.top() == 0x0202);

            END(res, os);
        }

        {
            SETUP("Return From Subroutine (0x00EE)");

            SET(chip8.memory, 0x00FF, 0x00EE);
            chip8.cycle(true);

            res &= ASSERT(chip8.pc == 0x0202);

            END(res, os);
        }

        {
            SETUP("Skip If Equal (0x3XNN)");

            SET(chip8.memory, 0x200, 0x3003);
            chip8.registers[0] = 2;
            chip8.pc = 0x200;
            chip8.cycle(true);
            res &= ASSERT(chip8.pc == 0x202);

            chip8.registers[0] = 3;
            chip8.pc = 0x200;
            chip8.cycle(true);
            res &= ASSERT(chip8.pc == 0x204);

            END(res, os);
        }

        {
            SETUP("Skip If Not Equal (0x4XNN)");

            SET(chip8.memory, 0x200, 0x4003);
            chip8.registers[0] = 3;
            chip8.pc = 0x200;
            chip8.cycle(true);
            res &= ASSERT(chip8.pc == 0x202);

            chip8.registers[0] = 2;
            chip8.pc = 0x200;
            chip8.cycle(true);
            res &= ASSERT(chip8.pc == 0x204);

            END(res, os);
        }

        {
            SETUP("Skip If Registers Equal (0x5XY0)");

            SET(chip8.memory, 0x200, 0x5010);
            chip8.registers[0] = 1;
            chip8.registers[1] = 2;
            chip8.pc = 0x200;
            chip8.cycle(true);
            res &= ASSERT(chip8.pc == 0x202);

            chip8.registers[0] = 1;
            chip8.registers[1] = 1;
            chip8.pc = 0x200;
            chip8.cycle(true);
            res &= ASSERT(chip8.pc == 0x204);

            END(res, os);
        }

        {
            SETUP("Skip If Registers Not Equal (0x9XY0)");

            SET(chip8.memory, 0x200, 0x9010);
            chip8.registers[0] = 1;
            chip8.registers[1] = 1;
            chip8.pc = 0x200;
            chip8.cycle(true);
            res &= ASSERT(chip8.pc == 0x202);

            chip8.registers[0] = 2;
            chip8.registers[1] = 1;
            chip8.pc = 0x200;
            chip8.cycle(true);
            res &= ASSERT(chip8.pc == 0x204);

            END(res, os);
        }

        {
            SETUP("Set (0x6XNN)");

            SET(chip8.memory, 0x200, 0x6303);
            chip8.pc = 0x200;
            chip8.cycle(true);
            res &= ASSERT(chip8.registers[3] == 0x03);

            END(res, os);
        }

        {
            SETUP("Add (0x7XNN)");

            SET(chip8.memory, 0x200, 0x7302);
            chip8.pc = 0x200;
            chip8.cycle(true);
            res &= ASSERT(chip8.registers[3] == 0x05);

            END(res, os);
        }

        {
            SETUP("Set Index (0xANNN)");

            SET(chip8.memory, 0x200, 0xA0BC);
            chip8.pc = 0x200;
            chip8.cycle(true);
            res &= ASSERT(chip8.I == 0xBC);

            END(res, os);
        }

        // TODO: test draw

        {
            SETUP("Random (0xCX0F)");

            SET(chip8.memory, 0x200, 0xC00F);
            chip8.registers[0] = 0xFF;
            chip8.pc = 0x200;
            chip8.cycle(true);
            // Make sure the mask work
            res &= ASSERT(!(chip8.registers[0] & 0xF0));

            END(res, os);
        }

        {
            SETUP("Set Register (0x8XY0)");

            chip8.registers[0] = 0x3;
            chip8.registers[1] = 0x4;

            SET(chip8.memory, 0x200, 0x8010);
            chip8.pc = 0x200;
            chip8.cycle(true);
            res &= ASSERT(chip8.registers[0] == 0x4);

            END(res, os);
        }

        {
            SETUP("Binary OR (0x8XY1)");

            chip8.registers[0] = 0x3;
            chip8.registers[1] = 0x4;

            SET(chip8.memory, 0x200, 0x8011);
            chip8.pc = 0x200;
            chip8.cycle(true);
            res &= ASSERT(chip8.registers[0] == 0x7);

            END(res, os);
        }

        {
            SETUP("Binary AND (0x8XY2)");

            chip8.registers[0] = 0x3;
            chip8.registers[1] = 0x4;

            SET(chip8.memory, 0x200, 0x8012);
            chip8.pc = 0x200;
            chip8.cycle(true);
            res &= ASSERT(chip8.registers[0] == 0x0);

            END(res, os);
        }

        {
            SETUP("Logical XOR (0x8XY3)");

            chip8.registers[0] = 0x3;
            chip8.registers[1] = 0x4;

            SET(chip8.memory, 0x200, 0x8013);
            chip8.pc = 0x200;
            chip8.cycle(true);
            res &= ASSERT(chip8.registers[0] == 0x7);

            END(res, os);
        }

        {
            SETUP("Add Registers (0x8XY4)");

            chip8.registers[0] = 0x3;
            chip8.registers[1] = 0x4;

            SET(chip8.memory, 0x200, 0x8014);
            chip8.pc = 0x200;
            chip8.cycle(true);
            res &= ASSERT(chip8.registers[0] == 0x7);

            // Test overflow
            chip8.registers[0] = 0xFF;
            chip8.registers[1] = 0x01;

            SET(chip8.memory, 0x200, 0x8014);
            chip8.pc = 0x200;
            chip8.cycle(true);
            res &= ASSERT(chip8.registers[0] == 0x0);
            res &= ASSERT(chip8.registers[0xF] == 0x1);

            END(res, os);
        }

        {
            SETUP("Subtract X-Y (0x8XY5)");

            chip8.registers[0] = 0x4;
            chip8.registers[1] = 0x3;

            SET(chip8.memory, 0x200, 0x8015);
            chip8.pc = 0x200;
            chip8.cycle(true);
            res &= ASSERT(chip8.registers[0] == 0x1);

            // Test underflow
            chip8.registers[0] = 0x03;
            chip8.registers[1] = 0x04;

            SET(chip8.memory, 0x200, 0x8015);
            chip8.pc = 0x200;
            chip8.cycle(true);
            res &= ASSERT(chip8.registers[0] == 0xFF);
            res &= ASSERT(chip8.registers[0xF] == 0x1);

            END(res, os);
        }

        {
            SETUP("Subtract Y-X (0x8XY7)");

            chip8.registers[0] = 0x3;
            chip8.registers[1] = 0x4;

            SET(chip8.memory, 0x200, 0x8017);
            chip8.pc = 0x200;
            chip8.cycle(true);
            res &= ASSERT(chip8.registers[0] == 0x1);

            // Test underflow
            chip8.registers[0] = 0x04;
            chip8.registers[1] = 0x03;

            SET(chip8.memory, 0x200, 0x8017);
            chip8.pc = 0x200;
            chip8.cycle(true);
            res &= ASSERT(chip8.registers[0] == 0xFF);
            res &= ASSERT(chip8.registers[0xF] == 0x1);

            END(res, os);
        }

        {
            SETUP("Shift Right (0x8XY6)");

            chip8.USE_LEGACY_SHIFT = false;
            chip8.registers[0] = 0x5;

            SET(chip8.memory, 0x200, 0x8016);
            chip8.pc = 0x200;
            chip8.cycle(true);
            res &= ASSERT(chip8.registers[0] == 0x2);
            res &= ASSERT(chip8.registers[0xF] == 0x1);

            chip8.USE_LEGACY_SHIFT = true;
            chip8.registers[1] = 0x5;

            SET(chip8.memory, 0x200, 0x8016);
            chip8.pc = 0x200;
            chip8.cycle(true);
            res &= ASSERT(chip8.registers[0] == 0x2);
            res &= ASSERT(chip8.registers[0xF] == 0x1);

            END(res, os);
        }

        {
            SETUP("Shift Left (0x8XYE)");

            chip8.USE_LEGACY_SHIFT = false;
            chip8.registers[0] = 0x8C;

            SET(chip8.memory, 0x200, 0x801E);
            chip8.pc = 0x200;
            chip8.cycle(true);
            res &= ASSERT(chip8.registers[0] == 0x18);
            res &= ASSERT(chip8.registers[0xF] == 0x1);

            chip8.USE_LEGACY_SHIFT = true;
            chip8.registers[1] = 0x2;

            SET(chip8.memory, 0x200, 0x801E);
            chip8.pc = 0x200;
            chip8.cycle(true);
            res &= ASSERT(chip8.registers[0] == 0x4);
            res &= ASSERT(chip8.registers[0xF] == 0x0);

            END(res, os);
        }

        {
            SETUP("Skip If Key (0xEX9E)");

            chip8.keystates = 0;
            chip8.registers[0] = 0xA;

            SET(chip8.memory, 0x200, 0xE09E);
            chip8.pc = 0x200;
            chip8.cycle(true);

            res &= ASSERT(chip8.pc == 0x202);

            chip8.keystates = (0x1 << 0xA);
            chip8.pc = 0x200;
            chip8.cycle(true);

            res &= ASSERT(chip8.pc == 0x204);

            END(res, os);
        }

        {
            SETUP("Skip If Not Key (0xEXA1)");

            chip8.keystates = 0;
            chip8.registers[0] = 0xA;

            SET(chip8.memory, 0x200, 0xE0A1);
            chip8.pc = 0x200;
            chip8.cycle(true);

            res &= ASSERT(chip8.pc == 0x204);

            chip8.keystates = (0x1 << 0xA);
            chip8.pc = 0x200;
            chip8.cycle(true);

            res &= ASSERT(chip8.pc == 0x202);

            END(res, os);
        }

        {
            SETUP("Set X to Delay Timer (0xFX07)");

            SET(chip8.memory, 0x200, 0xF007);
            chip8.delay_timer = 60;
            chip8.pc = 0x200;
            chip8.cycle(true);

            res &= ASSERT(chip8.registers[0] == 60);

            END(res, os);
        }

        {
            SETUP("Set Delay Timer to X (0xFX15)");

            chip8.delay_timer = 0;
            chip8.registers[0] = 60;
            SET(chip8.memory, 0x200, 0xF015);
            chip8.pc = 0x200;
            chip8.cycle(true);

            res &= ASSERT(chip8.delay_timer == 60);

            END(res, os);
        }

        {
            SETUP("Set Sound Timer to X (0xFX18)");

            chip8.sound_timer = 0;
            chip8.registers[0] = 60;
            SET(chip8.memory, 0x200, 0xF018);
            chip8.pc = 0x200;
            chip8.cycle(true);

            res &= ASSERT(chip8.sound_timer == 60);

            END(res, os);
        }

        {
            SETUP("Add X to Index (0xF01E)");

            chip8.USE_LEGACY_INDEX_ADD = false;
            chip8.registers[0] = 1;
            chip8.I = 0xFFFF;
            SET(chip8.memory, 0x200, 0xF01E);
            chip8.pc = 0x200;
            chip8.cycle(true);

            res &= ASSERT(chip8.I == 0);

            chip8.USE_LEGACY_INDEX_ADD = true;
            chip8.registers[0] = 1;
            chip8.I = 0xFFFF;
            SET(chip8.memory, 0x200, 0xF01E);
            chip8.pc = 0x200;
            chip8.cycle(true);

            res &= ASSERT(chip8.I == 0);
            res &= ASSERT(chip8.registers[0xF] == 1);

            END(res, os);
        }

        {
            SETUP("Get Key (0xFX0A)");

            chip8.keystates = 0;
            SET(chip8.memory, 0x200, 0xF00A);
            chip8.pc = 0x200;
            chip8.cycle(true);

            res &= ASSERT(chip8.pc == 0x200);
             
            chip8.keystates = (0x1 << 0xA);
            chip8.cycle(true);
            res &= ASSERT(chip8.pc == 0x202);

            END(res, os);
        }

        {
            SETUP("Font Character (0xFX29)");

            chip8.registers[0] = 0xA;
            SET(chip8.memory, 0x200, 0xF029);
            chip8.pc = 0x200;
            chip8.cycle(true);

            res &= ASSERT(chip8.I == 0x0082);

            END(res, os);
        }

        {
            SETUP("Binary Coded Decimal Conversion (0xFX033)");

            chip8.registers[0] = 0x9C;
            chip8.I = 0x00FF;
            SET(chip8.memory, 0x200, 0xF033);
            chip8.pc = 0x200;
            chip8.cycle(true);

            res &= ASSERT(chip8.memory[chip8.I] == 1);
            res &= ASSERT(chip8.memory[chip8.I+1] == 5);
            res &= ASSERT(chip8.memory[chip8.I+2] == 6);

            END(res, os);
        }

        {
            SETUP("Store (0xFX55)");

            chip8.USE_LEGACY_LOAD_STORE = false;
            chip8.registers[0] = 0x0;
            chip8.registers[1] = 0x2;
            chip8.registers[2] = 0x5;

            chip8.I = 0x0400;
            chip8.registers[0xA] = 0x2;
            SET(chip8.memory, 0x200, 0xFA55);
            chip8.pc = 0x200;
            chip8.cycle(true);

            res &= ASSERT(chip8.memory[chip8.I] == 0x0);
            res &= ASSERT(chip8.memory[chip8.I+1] == 0x2);
            res &= ASSERT(chip8.memory[chip8.I+2] == 0x5);

            chip8.USE_LEGACY_LOAD_STORE = true;
            chip8.pc = 0x200;
            chip8.cycle(true);

            res &= ASSERT(chip8.memory[0x400] == 0x0);
            res &= ASSERT(chip8.memory[0x401] == 0x2);
            res &= ASSERT(chip8.memory[0x402] == 0x5);
            res &= ASSERT(chip8.I == 0x403);

            END(res, os);
        }

        {
            SETUP("Load (0xFX65)");

            chip8.USE_LEGACY_LOAD_STORE = false;

            chip8.I = 0x400;
            chip8.registers[0xA] = 0x2;
            SET(chip8.memory, 0x200, 0xFA65);
            chip8.memory[0x400] = 0x3;
            chip8.memory[0x401] = 0x7;
            chip8.memory[0x402] = 0xA;
            chip8.pc = 0x200;
            chip8.cycle(true);

            res &= ASSERT(chip8.registers[0] == 0x3);
            res &= ASSERT(chip8.registers[1] == 0x7);
            res &= ASSERT(chip8.registers[2] == 0xA);

            chip8.USE_LEGACY_LOAD_STORE = true;
            chip8.pc = 0x200;
            chip8.cycle(true);

            res &= ASSERT(chip8.registers[0] == 0x3);
            res &= ASSERT(chip8.registers[1] == 0x7);
            res &= ASSERT(chip8.registers[2] == 0xA);
            res &= ASSERT(chip8.I == 0x403);

            END(res, os);
        }

        os << std::endl;
    }

    void opcode(std::string const &op) {
        error_flag = false;
        os << ANSI::BLUE << std::left << std::setw(48) <<  ": " + op; 
    }
};

 std::string const ANSI {"\033"};
