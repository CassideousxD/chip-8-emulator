#include "./chip8.h"
#include <cstring>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdlib>

// CHIP-8 Font Set
static const uint8_t fontset[80] =
{
    0xF0,0x90,0x90,0x90,0xF0, // 0
    0x20,0x60,0x20,0x20,0x70, // 1
    0xF0,0x10,0xF0,0x80,0xF0, // 2
    0xF0,0x10,0xF0,0x10,0xF0, // 3
    0x90,0x90,0xF0,0x10,0x10, // 4
    0xF0,0x80,0xF0,0x10,0xF0, // 5
    0xF0,0x80,0xF0,0x90,0xF0, // 6
    0xF0,0x10,0x20,0x40,0x40, // 7
    0xF0,0x90,0xF0,0x90,0xF0, // 8
    0xF0,0x90,0xF0,0x10,0xF0, // 9
    0xF0,0x90,0xF0,0x90,0x90, // A
    0xE0,0x90,0xE0,0x90,0xE0, // B
    0xF0,0x80,0x80,0x80,0xF0, // C
    0xE0,0x90,0x90,0x90,0xE0, // D
    0xF0,0x80,0xF0,0x80,0xF0, // E
    0xF0,0x80,0xF0,0x80,0x80  // F
};

Chip8::Chip8()
{
    memset(memory, 0, sizeof(memory));

    memset(V, 0, sizeof(V));

    memset(stack, 0, sizeof(stack));

    memset(keypad, 0, sizeof(keypad));

    memset(display, 0, sizeof(display));

    I = 0;

    pc = 0x200;

    sp = 0;

    opcode = 0;

    delayTimer = 0;
    soundTimer = 0;

    for (size_t  i = 0; i < 80; i++)
    {
        memory[0x50 + i] = fontset[i];
    }
}

void Chip8::printstate() const
{
    std::cout << "===== CHIP-8 STATE =====\n";

    std::cout << "PC: 0x"
              << std::hex
              << std::uppercase
              << pc
              << "\n";

    std::cout << "I : 0x"
              << std::hex
              << std::uppercase
              << I
              << "\n";

    std::cout << "SP: "
              << std::dec
              << static_cast<int>(sp)
              << "\n";

    std::cout << "Delay Timer: "
              << static_cast<int>(delayTimer)
              << "\n";

    std::cout << "Sound Timer: "
              << static_cast<int>(soundTimer)
              << "\n";

    std::cout << "\nRegisters:\n";

    for(int i = 0; i < 16; i++)
    {
        std::cout << "V"
                  << std::hex
                  << std::uppercase
                  << i
                  << ": 0x"
                  << std::setw(2)
                  << std::setfill('0')
                  << static_cast<int>(V[i])
                  << "\n";
    }

    std::cout << "\nFirst 10 Font Bytes:\n";

    for(int i = 0; i < 10; i++)
    {
        std::cout << "Memory[0x"
                  << std::hex
                  << (0x50 + i)
                  << "] = 0x"
                  << static_cast<int>(memory[0x50 + i])
                  << "\n";
    }

    std::cout << "\nFirst 20 ROM Bytes:\n";

    for(int i = 0; i < 20; i++)
    {
        std::cout << std::hex
                << static_cast<int>(memory[0x200 + i])
                << '\n';
    }

    std::cout << "========================\n";
}

bool Chip8::loadROM(const std::string& filename)
{
    std::ifstream rom(filename, std::ios::binary);

    if (!rom)
    {
        return false;
    }

    rom.seekg(0, std::ios::end);
    std::streamsize size = rom.tellg();
    std::cout << "ROM size: " << size << " bytes\n";
    rom.seekg(0, std::ios::beg);

    if (size > (4096 - 0x200))
    {
        return false;
    }

    rom.read(
        reinterpret_cast<char*>(&memory[0x200]),
        size
    );

    return true;
}

void Chip8::Cycle()
{
    // Fetch
    opcode = (memory[pc] << 8u) | memory[pc + 1];

    pc += 2;

    // Decode
    switch (opcode & 0xF000)
    {
        case 0x0000:
        {
            switch (opcode)
            {
                case 0x00E0:
                {
                    memset(display, 0, sizeof(display));
                    std::cout << "CLS" << std::endl;
                    break;
                }

                case 0x00EE:
                {
                    sp--;
                    pc = stack[sp];
                    break;
                }

                default:
                    break;
            }

            break;
        }

        case 0x1000:
        {
            uint16_t NNN = opcode & 0x0FFF;

            /*std::cout << "JP "
                    << std::hex << NNN
                    << std::endl;*/

            pc = NNN;

            break;
        }

        case 0x2000:
        {
            uint16_t NNN = opcode & 0x0FFF;

            stack[sp] = pc;
            sp++;

            pc = NNN;

            break;
        }

        case 0x3000:
        {
            uint8_t X = (opcode & 0x0F00) >> 8;
            uint8_t NN = opcode & 0x00FF;

            if (V[X] == NN)
            {
                pc += 2;
            }

            break;
        }

        case 0x4000:
        {
            uint8_t X = (opcode & 0x0F00) >> 8;
            uint8_t NN = opcode & 0x00FF;

            if (V[X] != NN)
            {
                pc += 2;
            }

            break;
        }

        case 0x5000:
        {
            if ((opcode & 0x000F) == 0)
            {
                uint8_t X = (opcode & 0x0F00) >> 8;
                uint8_t Y = (opcode & 0x00F0) >> 4;

                if (V[X] == V[Y])
                {
                    pc += 2;
                }
            }

            break;
        }

        case 0x6000:
        {  uint8_t X = (opcode & 0x0F00) >> 8;
            uint8_t NN = opcode & 0x00FF;

            V[X] = NN;

            /*std::cout
                << "LD V"
                << std::hex << (int)X
                << ", "
                << (int)NN
                << std::endl;*/

            break;
        }

        case 0x7000:
        {
            uint8_t X = (opcode & 0x0F00) >> 8;
            uint8_t NN = opcode & 0x00FF;

            V[X] += NN;

            /*std::cout << "ADD V"
                    << std::hex << (int)X
                    << ", "
                    << (int)NN
                    << std::endl;*/

            break;
        }

        case 0x8000:
        {
            uint8_t X = (opcode & 0x0F00) >> 8;
            uint8_t Y = (opcode & 0x00F0) >> 4;

            switch (opcode & 0x000F)
            {
                case 0x0:
                {
                    V[X] = V[Y];
                    break;
                }

                case 0x1:
                {
                    V[X] |= V[Y];
                    break;
                }

                case 0x2:
                {
                    V[X] &= V[Y];
                    break;
                }

                case 0x3:
                {
                    V[X] ^= V[Y];
                    break;
                }

                case 0x4:
                {
                    uint16_t sum = V[X] + V[Y];
                    if (sum > 255)
                    {
                        V[0xF] = 1;
                    }
                    else
                    {
                        V[0xF] = 0;
                    }

                    V[X] = sum & 0xFF;

                    break;
                }

                case 0x5:
                {
                    if (V[X] >= V[Y])
                    {
                        V[0xF] = 1;
                    }
                    else
                    {
                        V[0xF] = 0;
                    }

                    V[X] = V[X] - V[Y];

                    break;
                }

                case 0x6:
                {
                    V[0xF] = V[X] & 0x1;
                    V[X] >>= 1;
                    break;
                }

                case 0x7:
                {
                    V[0xF] = (V[Y] >= V[X]);
                    V[X] = V[Y] - V[X];
                    break;
                }

                case 0xE:
                {
                    V[0xF] = (V[X] & 0x80) >> 7;
                    V[X] <<= 1;
                    break;
                }

                default:
                {
                    std::cout << "Unknown 8XY opcode: "
                            << std::hex << opcode
                            << std::endl;
                    break;
                }
            }

            break;
        }

        case 0x9000:
        {
            if ((opcode & 0x000F) == 0)
            {
                uint8_t X = (opcode & 0x0F00) >> 8;
                uint8_t Y = (opcode & 0x00F0) >> 4;

                if (V[X] != V[Y])
                {
                    pc += 2;
                }
            }

            break;
        }

        case 0xA000:
        {
            uint16_t NNN = opcode & 0x0FFF;

            I = NNN;

            break;
        }

        case 0xB000:
        {
            uint16_t NNN = opcode & 0x0FFF;
            pc = NNN + V[0];
            break;
        }

        case 0xC000:
        {
            uint8_t X = (opcode & 0x0F00) >> 8;
            uint8_t NN = opcode & 0x00FF;

            V[X] = (rand() % 256) & NN;

            break;
        }

        case 0xD000:
        {
            uint8_t X = V[(opcode & 0x0F00) >> 8];
            uint8_t Y = V[(opcode & 0x00F0) >> 4];
            uint8_t height = opcode & 0x000F;

            V[0xF] = 0;

            for (unsigned int row = 0; row < height; row++)
            {
                uint8_t spriteByte = memory[I + row];

                for (unsigned int col = 0; col < 8; col++)
                {
                    uint8_t spritePixel = spriteByte & (0x80 >> col);

                    if (spritePixel)
                    {
                        uint16_t xPos = (X + col) % 64;
                        uint16_t yPos = (Y + row) % 32;

                        uint16_t index = yPos * 64 + xPos;

                        if (display[index] == 1)
                        {
                            V[0xF] = 1;
                        }

                        display[index] ^= 1;
                    }
                }
            }

            drawFlag = true;

            break;
        }

        case 0xE000:
        {
            uint8_t X = (opcode & 0x0F00) >> 8;

            switch (opcode & 0x00FF)
            {
                case 0x9E:
                {
                    if (keypad[V[X]])
                    {
                        pc += 2;
                    }

                    break;
                }

                case 0xA1:
                {
                    if (!keypad[V[X]])
                    {
                        pc += 2;
                    }

                    break;
                }

                default:
                {
                    std::cout
                        << "Unknown opcode: "
                        << std::hex
                        << opcode
                        << std::endl;
                    break;
                }
            }

            break;
        }

        case 0xF000:
        {
            uint8_t X = (opcode & 0x0F00) >> 8;

            switch (opcode & 0x00FF)
            {

                case 0x07:
                {
                    V[X] = delayTimer;
                    break;
                }

                case 0x15:
                {
                    delayTimer = V[X];
                    break;
                }

                case 0x18:
                {
                    soundTimer = V[X];
                    break;
                }

                case 0x1E:
                {
                    I += V[X];
                    break;
                }

                case 0x29:
                {
                    I = 0x50 + (V[X] * 5);
                    break;
                }

                case 0x33:
                {
                    memory[I]     = V[X] / 100;
                    memory[I + 1] = (V[X] / 10) % 10;
                    memory[I + 2] = V[X] % 10;

                    break;
                }

                case 0x55:
                {
                    for (int i = 0; i <= X; i++)
                    {
                        memory[I + i] = V[i];
                    }

                    break;
                }

                case 0x65:
                {
                    for (int i = 0; i <= X; i++)
                    {
                        V[i] = memory[I + i];
                    }

                    break;
                }

                

                case 0x0A:
                {
                    bool keyPressed = false;

                    for (int i = 0; i < 16; i++)
                    {
                        if (keypad[i])
                        {
                            V[X] = i;
                            keyPressed = true;
                            break;
                        }
                    }

                    if (!keyPressed)
                    {
                        pc -= 2;
                    }

                    break;
                }

                default:
                {
                    std::cout
                        << "Unknown FX opcode: "
                        << std::hex
                        << opcode
                        << std::endl;
                    break;
                }
            }

            break;
        }

        default:
            std::cout << "Unknown opcode: "
                      << std::hex << opcode << std::endl;
            break;
    }
}

const uint8_t* Chip8::GetDisplay() const
{
    return display;
}

bool Chip8::NeedsRedraw() const
{
    return drawFlag;
}

void Chip8::ClearDrawFlag()
{
    drawFlag = false;
}

void Chip8::UpdateTimers()
{
    if (delayTimer > 0)
    {
        delayTimer--;
    }

    if (soundTimer > 0)
    {
        soundTimer--;
    }
}

void Chip8::KeyDown(uint8_t key)
{
    keypad[key] = 1;
}

void Chip8::KeyUp(uint8_t key)
{
    keypad[key] = 0;
}

uint8_t Chip8::GetSoundTimer() const
{
    return soundTimer;
}