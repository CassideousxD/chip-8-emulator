#pragma once
#include <cstdint>
#include <string>

class Chip8
{
public:
    Chip8();
    void printstate() const;
    bool loadROM(const std::string& filename);
    void Cycle();
    void UpdateTimers();

    void KeyDown(uint8_t key);
    void KeyUp(uint8_t key);

    const uint8_t* GetDisplay() const;
    bool NeedsRedraw() const;
    void ClearDrawFlag();

    uint8_t GetSoundTimer() const;

private:

    uint8_t memory[4096];
    uint16_t opcode;
    uint8_t V[16];
    uint16_t I;
    uint16_t pc;
    uint16_t stack[16];
    uint8_t sp;
    uint8_t delayTimer;
    uint8_t soundTimer;
    uint8_t keypad[16];
    uint8_t display[64 * 32];
    bool drawFlag = false;
};
