# CHIP-8 Emulator

A CHIP-8 emulator written in modern C++ using SDL3 and SDL3_ttf. This project emulates the original CHIP-8 virtual machine, supporting graphics, input, timers, audio, ROM management, and an interactive SDL-based ROM selection menu.

---

## Features

* Complete CHIP-8 CPU implementation
* ROM loading support
* SDL3 graphics rendering
* Keyboard input handling
* Delay timer support
* Sound timer and audio beeper
* Fullscreen mode
* Pause and resume functionality
* Reset current ROM
* Automatic ROM discovery
* Scrollable ROM selection menu
* Return to menu without restarting emulator
* Support for CHIP-8 test ROMs
* Playable Pong and other CHIP-8 games

---

## Controls

### Menu

| Key        | Action              |
| ---------- | ------------------- |
| Up Arrow   | Move Selection Up   |
| Down Arrow | Move Selection Down |
| Enter      | Launch Selected ROM |

### Emulator

| Key | Action            |
| --- | ----------------- |
| Esc | Return to Menu    |
| H   | Reset Current ROM |
| P   | Pause / Resume    |
| G   | Toggle Fullscreen |

---

## Project Structure

```text
CHIP-8/
├── ROM/
│   ├── Pong.ch8
│   ├── IBM Logo.ch8
│   ├── BC_test.ch8
│   └── ...
│
├── Documents/
│   └── README.md
|   ├── DEVELOPMENT_LOG.md
|   ├── LESSONS_LEARNED.md
│   └── SCREENSHOTS
|           ├── Emulator-Menu.png
│           ├── Fullscreen-Preview.png
│           └── Windowed-Preview.png
│
├── src/
│   ├── main.cpp
│   ├── chip8.cpp
│   └── chip8.h
│
└── CMakeLists.txt
```

---

## Building and Running

### Requirements

* C++17 or newer
* CMake 3.15+
* SDL3
* SDL3_ttf

### Installing Dependencies (macOS)

```bash
brew install sdl3
brew install sdl3_ttf
```

### Build

From the project root:

```bash
mkdir build
cd build

cmake ..
cmake --build .
```

### Run

```bash
./CHIP8
```
---

## Technologies Used

* C++
* SDL3
* SDL3_ttf
* CMake

---

## Learning Outcomes

This project provided practical experience with:

* CPU emulation
* Fetch-Decode-Execute cycles
* Opcode decoding
* Memory management
* Stack operations
* Graphics rendering
* Event-driven input handling
* Audio generation
* Emulator architecture
* CMake project management

---

## Current Status

CHIP-8 Emulator v1.0 Complete

Features implemented:

* CPU
* Graphics
* Input
* Audio
* Timers
* ROM Browser
* SDL Menu
* Fullscreen
* Reset System

---

## Future Improvements

* Save states
* Custom key mapping
* Emulator settings menu
* SCHIP support
* Debugger
* ROM metadata and previews

---

## Author

Roshan Kumar

Built as a learning project to understand emulator development, computer architecture, graphics rendering, input systems, and audio programming.
