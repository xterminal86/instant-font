#include "instant-font.h"

bool IsRunning = true;

const std::vector<std::string> LoremIpsum =
{
  "One, two, three, four.",
  "ax += 5, by = 6%, dx/dy = 33$, i++, j--",
  "db qp ad ab ,.- dx dz dy",
  "#!/bin/bash",
  "A quick brown fox jumps over a lazy dog.",
  "ls -la ; echo \"test \\n\" && rm -rf /",
  "SELECT * FROM table WHERE `id` >= 42 AND `rating` = '30%'",
  R"({ "root" : { "key1" : "value" }, { "key2" : 33 } })",
};

// =============================================================================

void HandleEvent(const SDL_Event& evt)
{
  const uint8_t* kb = SDL_GetKeyboardState(nullptr);
  if (kb[SDL_SCANCODE_ESCAPE])
  {
    IsRunning = false;
  }
}

// =============================================================================

int main(int argc, char* argv[])
{
  if (SDL_Init(SDL_INIT_VIDEO) != 0)
  {
    printf("SDL_Init Error: %s\n", SDL_GetError());
    return 1;
  }

  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);

  SDL_Window* window = SDL_CreateWindow("Instant Font",
                                        50, 50,
                                        800, 600,
                                        SDL_WINDOW_SHOWN);

  SDL_Renderer* r = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (r == nullptr)
  {
    SDL_Log("Failed to create renderer with SDL_RENDERER_ACCELERATED"
            " - falling back to software");

    r = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (r == nullptr)
    {
      SDL_Log("Failed to create renderer: %s", SDL_GetError());
      return false;
    }
  }

  if (not IF::Instance().Init(r))
  {
    return 1;
  }

  SDL_Event evt;

  uint64_t dt = 0;

  while (IsRunning)
  {
    uint64_t before = SDL_GetTicks();

    while (SDL_PollEvent(&evt))
    {
      HandleEvent(evt);
    }

    SDL_RenderClear(r);

    SDL_SetRenderDrawColor(r, 32, 32, 32, 255);

    static SDL_Rect box;
    box.x = 0;
    box.y = 0;
    box.w = 750;
    box.h = 280;

    SDL_RenderFillRect(r, &box);

    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);

    IF::Instance().ShowFontBitmap();

    int lineInd = 0;
    for (auto& line : LoremIpsum)
    {
      IF::Instance().Print(0, 300 + lineInd, line);
      lineInd += 10;
    }

    IF::Instance().Print(650, 290, "Simple print");
    IF::Instance().Print(650, 300, "Colored print", 0xFFFF00);

    IF::Instance().Print(780, 320,
                         "Scale = 1.5",
                         0xFFFFFF,
                         IF::TextAlignment::RIGHT,
                         1.5);

    IF::Instance().Print(780, 340,
                         "Scale = 2",
                         0xFFFFFF,
                         IF::TextAlignment::RIGHT,
                         2.0);

    IF::Instance().Print(780, 360,
                         "Scale = 3",
                         0xFFFFFF,
                         IF::TextAlignment::RIGHT,
                         3.0);

    IF::Instance().Printf(780, 420,
                          IF::TextParams::Set(0xFFFFFF,
                                              IF::TextAlignment::RIGHT,
                                              2.0),
                          "Delta time = %llu", dt);

    IF::Instance().Print(570, 440,
                         "Left aligned",
                         0xFF0000,
                         IF::TextAlignment::LEFT,
                         2.0);

    IF::Instance().Print(570, 460,
                         "Right aligned",
                         0x00FF00,
                         IF::TextAlignment::RIGHT,
                         2.0);

    IF::Instance().Print(570, 480,
                         "Centered",
                         0x0088FF,
                         IF::TextAlignment::CENTER,
                         2.0);


    IF::Instance().Print(400, 520,
                         "Non-printable: Дbё",
                         0xFFFFFF,
                         IF::TextAlignment::LEFT,
                         2.0);

    SDL_RenderPresent(r);

    dt = SDL_GetTicks() - before;
  }

  SDL_Log("Goodbye!");

  SDL_Quit();

  return 0;
}
