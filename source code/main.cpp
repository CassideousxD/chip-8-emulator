#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>
#include <iostream>
#include "./chip8.h"
#include <chrono>
#include <cmath>
#include <filesystem>
#include <vector>
#include <SDL3_ttf/SDL_ttf.h>

bool beepOn = false;
double phase = 0.0;

const int VIDEO_WIDTH = 64;
const int VIDEO_HEIGHT = 32;
const int SCALE = 10;

void AudioCallback(void* userdata,
                   SDL_AudioStream* stream,
                   int additional_amount,
                   int total_amount)
{
    const int sampleRate = 44100;
    const double frequency = 440.0;

    float samples[512];

    for (int i = 0; i < 512; i++)
    {
        if (beepOn)
        {
            samples[i] =
                (phase < 0.5) ? 0.25f : -0.25f;

            phase += frequency / sampleRate;

            if (phase >= 1.0)
            {
                phase -= 1.0;
            }
        }
        else
        {
            samples[i] = 0.0f;
        }
    }

    SDL_PutAudioStreamData(
        stream,
        samples,
        sizeof(samples)
    );
}

int main()
{
    Chip8 chip8;

    std::vector<std::string> roms;

    for (const auto& entry :
        std::filesystem::directory_iterator("../ROM"))
    {
        if (entry.is_regular_file())
        {
            roms.push_back(entry.path().string());
        }
    }

    if (roms.empty())
    {
        std::cout
            << "No ROMs found!"
            << std::endl;

        return 1;
    }

    std::sort(
        roms.begin(),
        roms.end()
    );



    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
    {
        std::cout << "SDL Init Failed\n";
        return 1;
    }

    if (!TTF_Init())
    {
        std::cout
            << "TTF Init Failed: "
            << SDL_GetError()
            << std::endl;

        return 1;
    }

    TTF_Font* font =
    TTF_OpenFont(
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        10
    );

    if (!font)
    {
        std::cout
            << "Font load failed!"
            << std::endl;

        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "CHIP-8",
        VIDEO_WIDTH * SCALE,
        VIDEO_HEIGHT * SCALE,
        0
    );

    SDL_Renderer* renderer =
        SDL_CreateRenderer(window, nullptr);

    SDL_SetRenderLogicalPresentation(
        renderer,
        64,
        32,
        SDL_LOGICAL_PRESENTATION_INTEGER_SCALE
    );

    SDL_AudioSpec spec{};
    spec.format = SDL_AUDIO_F32;
    spec.channels = 1;
    spec.freq = 44100;

    SDL_AudioStream* audioStream =
        SDL_OpenAudioDeviceStream(
            SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
            &spec,
            AudioCallback,
            nullptr
        );

    if (!audioStream)
    {
        std::cout
            << "Audio failed: "
            << SDL_GetError()
            << std::endl;
    }

    SDL_ResumeAudioStreamDevice(
        audioStream
    );

    bool running = true;
    bool paused = false;
    bool fullscreen = false;
    bool inMenu = true;
    int selectedRom = 0;
    int scrollOffset = 0;
    std::string currentRomPath;
    const int visibleItems = 5;
    SDL_Event event;

    auto lastTimerUpdate =
    std::chrono::high_resolution_clock::now();

    while (running)
    {
        auto now =
        std::chrono::high_resolution_clock::now();

        auto elapsed =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                now - lastTimerUpdate
            );

        if (elapsed.count() >= 16)
        {
            chip8.UpdateTimers();
            
            beepOn =chip8.GetSoundTimer() > 0;

            lastTimerUpdate = now;
        }

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                running = false;
            }

            if (event.type == SDL_EVENT_KEY_DOWN)
            {
                if (event.key.key == SDLK_P)
                {
                    paused = !paused;
                }
            }

            if (event.type == SDL_EVENT_KEY_DOWN)
            {
                switch (event.key.key)
                {
                    case SDLK_1: chip8.KeyDown(0x1); break;
                    case SDLK_2: chip8.KeyDown(0x2); break;
                    case SDLK_3: chip8.KeyDown(0x3); break;
                    case SDLK_4: chip8.KeyDown(0xC); break;

                    case SDLK_Q: chip8.KeyDown(0x4); break;
                    case SDLK_W: chip8.KeyDown(0x5); break;
                    case SDLK_E: chip8.KeyDown(0x6); break;
                    case SDLK_R: chip8.KeyDown(0xD); break;

                    case SDLK_A: chip8.KeyDown(0x7); break;
                    case SDLK_S: chip8.KeyDown(0x8); break;
                    case SDLK_D: chip8.KeyDown(0x9); break;
                    case SDLK_F: chip8.KeyDown(0xE); break;

                    case SDLK_Z: chip8.KeyDown(0xA); break;
                    case SDLK_X: chip8.KeyDown(0x0); break;
                    case SDLK_C: chip8.KeyDown(0xB); break;
                    case SDLK_V: chip8.KeyDown(0xF); break;
                }

                if (event.key.key == SDLK_G)
                {
                    fullscreen = !fullscreen;

                    SDL_SetWindowFullscreen(
                        window,
                        fullscreen
                    );
                }

                if (inMenu)
                {
                    switch (event.key.key)
                    {
                        case SDLK_UP:
                            selectedRom--;

                            if (selectedRom < 0)
                            {
                                selectedRom =
                                    static_cast<int>(roms.size()) - 1;
                                scrollOffset =std::max(0,static_cast<int>(roms.size()) - visibleItems);
                            }
                            if (selectedRom < scrollOffset)
                            {
                                scrollOffset--;
                            }
                            break;

                        case SDLK_DOWN:
                            selectedRom++;

                            if (selectedRom >= static_cast<int>(roms.size()))
                            {
                                selectedRom = 0;
                                scrollOffset = 0;
                            }
                            if (selectedRom >= scrollOffset + visibleItems)
                            {
                                scrollOffset++;
                            }
                            break;

                        case SDLK_RETURN:
                        {
                            chip8 = Chip8();

                            if (!chip8.loadROM(roms[selectedRom]))
                            {
                                std::cout
                                    << "Failed to load ROM!"
                                    << std::endl;
                            }
                            else
                            {
                                currentRomPath = roms[selectedRom];
                                std::string romName =
                                    std::filesystem::path(
                                        currentRomPath
                                    ).stem().string();

                                std::string title =
                                    "CHIP-8 - " + romName;

                                SDL_SetWindowTitle(
                                    window,
                                    title.c_str()
                                );
                                inMenu = false;
                                SDL_SetRenderLogicalPresentation(renderer,64,32,SDL_LOGICAL_PRESENTATION_INTEGER_SCALE);
                            }

                            break;
                        }
                    }
                }

                if (!inMenu)
                {
                    if (event.key.key == SDLK_ESCAPE)
                    {
                        chip8 = Chip8();
                        SDL_SetRenderLogicalPresentation(
                            renderer,
                            640,
                            320,
                            SDL_LOGICAL_PRESENTATION_DISABLED
                        );

                        selectedRom = 0;
                        scrollOffset = 0;

                        SDL_SetWindowTitle(
                            window,
                            "CHIP-8 Emulator"
                        );

                        inMenu = true;                    
                    }

                    if (event.key.key == SDLK_H)
                    {
                        chip8 = Chip8();
                        chip8.loadROM(currentRomPath);
                        std::cout<< "ROM Reset"<< std::endl;
                    }
                }
            }

            if (event.type == SDL_EVENT_KEY_UP)
            {
                switch (event.key.key)
                {
                    case SDLK_1: chip8.KeyUp(0x1); break;
                    case SDLK_2: chip8.KeyUp(0x2); break;
                    case SDLK_3: chip8.KeyUp(0x3); break;
                    case SDLK_4: chip8.KeyUp(0xC); break;

                    case SDLK_Q: chip8.KeyUp(0x4); break;
                    case SDLK_W: chip8.KeyUp(0x5); break;
                    case SDLK_E: chip8.KeyUp(0x6); break;
                    case SDLK_R: chip8.KeyUp(0xD); break;

                    case SDLK_A: chip8.KeyUp(0x7); break;
                    case SDLK_S: chip8.KeyUp(0x8); break;
                    case SDLK_D: chip8.KeyUp(0x9); break;
                    case SDLK_F: chip8.KeyUp(0xE); break;

                    case SDLK_Z: chip8.KeyUp(0xA); break;
                    case SDLK_X: chip8.KeyUp(0x0); break;
                    case SDLK_C: chip8.KeyUp(0xB); break;
                    case SDLK_V: chip8.KeyUp(0xF); break;
                }
            }
        }

        if (inMenu)
        {

            SDL_SetRenderLogicalPresentation(
                renderer,
                640,
                320,
                SDL_LOGICAL_PRESENTATION_DISABLED
            );

            SDL_SetRenderDrawColor(
                renderer,
                20,
                20,
                20,
                255
            );

            SDL_RenderClear(renderer);

            int startY = 4;

            SDL_Color titleColor =
            {
                255,
                255,
                255,
                255
            };

            SDL_Surface* titleSurface =
                TTF_RenderText_Blended(
                    font,
                    "CHIP-8 EMULATOR",
                    0,
                    titleColor
                );

            SDL_Texture* titleTexture =
                SDL_CreateTextureFromSurface(
                    renderer,
                    titleSurface
                );

            SDL_FRect titleRect =
            {
                (640.0f - titleSurface->w) / 2.0f,
                20.0f,
                static_cast<float>(titleSurface->w),
                static_cast<float>(titleSurface->h)
            };

            SDL_RenderTexture(
                renderer,
                titleTexture,
                nullptr,
                &titleRect
            );

            for (int visibleIndex = 0;visibleIndex < visibleItems;visibleIndex++)
            {
                int i = scrollOffset + visibleIndex;

                if (i >= static_cast<int>(roms.size()))
                {
                    break;
                }

                std::string romName =
                    std::filesystem::path(
                        roms[i]
                    ).stem().string();

                std::string displayName;

                if (static_cast<int>(i) == selectedRom)
                {
                    displayName = "> " + romName;
                }
                else
                {
                    displayName = "  " + romName;
                }

                SDL_Color textColor;

                if (static_cast<int>(i) == selectedRom)
                {
                    textColor =
                    {
                        255,
                        255,
                        255,
                        255
                    };
                }
                else
                {
                    textColor =
                    {
                        170,
                        170,
                        170,
                        255
                    };
                }

                SDL_Surface* textSurface =
                    TTF_RenderText_Blended(
                        font,
                        displayName.c_str(),
                        0,
                        textColor
                    );

                SDL_Texture* textTexture =
                    SDL_CreateTextureFromSurface(
                        renderer,
                        textSurface
                    );

                SDL_FRect textRect =
                {
                    80.0f,
                    80.0f + visibleIndex * 40.0f,                    
                    static_cast<float>(textSurface->w),
                    static_cast<float>(textSurface->h)
                };

                SDL_RenderTexture(
                    renderer,
                    textTexture,
                    nullptr,
                    &textRect
                );

                                SDL_Color footerColor =
                {
                    180,
                    180,
                    180,
                    255
                };

                SDL_Surface* footerSurface =
                    TTF_RenderText_Blended(
                        font,
                        "UP/DOWN Select  ENTER Launch",
                        0,
                        footerColor
                    );

                SDL_Texture* footerTexture =
                    SDL_CreateTextureFromSurface(
                        renderer,
                        footerSurface
                    );

                SDL_FRect footerRect =
                {
                    50.0f,
                    280.0f,
                    static_cast<float>(footerSurface->w),
                    static_cast<float>(footerSurface->h)
                };

                SDL_RenderTexture(
                    renderer,
                    footerTexture,
                    nullptr,
                    &footerRect
                );

                SDL_Surface* footerSurface2 =
                    TTF_RenderText_Blended(
                        font,
                        "ESC Menu  H Reset  G Fullscreen  P Pause",
                        0,
                        footerColor
                    );

                SDL_Texture* footerTexture2 =
                    SDL_CreateTextureFromSurface(
                        renderer,
                        footerSurface2
                    );

                SDL_FRect footerRect2 =
                {
                    50.0f,
                    300.0f,
                    static_cast<float>(footerSurface2->w),
                    static_cast<float>(footerSurface2->h)
                };

                SDL_RenderTexture(
                    renderer,
                    footerTexture2,
                    nullptr,
                    &footerRect2
                );

                SDL_DestroyTexture(
                    footerTexture2
                );

                SDL_DestroySurface(
                    footerSurface2
                );

                SDL_DestroyTexture(
                    footerTexture
                );

                SDL_DestroySurface(
                    footerSurface
                );

                SDL_DestroyTexture(
                    textTexture
                );

                SDL_DestroySurface(
                    textSurface
                );

                SDL_DestroyTexture(
                    titleTexture
                );

                SDL_DestroySurface(
                    titleSurface
                );
            }

            SDL_RenderPresent(renderer);

            SDL_Delay(16);

            continue;
        }

        // Run CPU
       if (!paused)
        {
            for (int i = 0; i < 2; i++)
            {
                chip8.Cycle();
            }
        }

        // Draw only if screen changed
        if (chip8.NeedsRedraw())
        {
            const uint8_t* display = chip8.GetDisplay();

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

            for (int y = 0; y < VIDEO_HEIGHT; y++)
            {
                for (int x = 0; x < VIDEO_WIDTH; x++)
                {
                    if (display[y * VIDEO_WIDTH + x])
                    {
                        SDL_FRect pixelRect =
                        {
                            static_cast<float>(x),
                            static_cast<float>(y),
                            1.0f,
                            1.0f
                        };

                        SDL_RenderFillRect(
                            renderer,
                            &pixelRect
                        );
                    }
                }
            }

            SDL_RenderPresent(renderer);

            chip8.ClearDrawFlag();
        }

        SDL_Delay(2);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_DestroyAudioStream(audioStream);
    SDL_Quit();

    return 0;
}