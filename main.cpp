#include "instant-font.h"

bool IsRunning = true;

const std::vector<std::string> LoremIpsum =
{
  "Lorem ipsum dolor sit amet," ,
  "consectetur adipiscing elit,",
  "sed do eiusmod tempor incididunt",
  "ut labore et dolore magna aliqua.",
  "Ut enim ad minim veniam, quis",
  "nostrud exercitation ullamco",
  "laboris nisi ut aliquip ex ea",
  "commodo consequat. Duis aute irure",
  "dolor in reprehenderit in voluptate",
  "velit esse cillum dolore eu fugiat",
  "nulla pariatur. Excepteur sint",
  "occaecat cupidatat non proident,",
  "sunt in culpa qui officia deserunt",
  "mollit anim id est laborum."
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

  SDL_Window* window = SDL_CreateWindow("Printer test",
                                        0, 0,
                                        1280, 720,
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

  IF::Instance().Init(r);

  SDL_SetRenderDrawColor(r, 0, 0, 0, 255);

  SDL_Event evt;

  while (IsRunning)
  {
    while (SDL_PollEvent(&evt))
    {
      HandleEvent(evt);
    }

    SDL_RenderClear(r);

    IF::Instance().DumpTexture();

    int lineInd = 0;
    for (auto& line : LoremIpsum)
    {
      IF::Instance().Print(0, 300 + lineInd, line);
      lineInd += 9;
    }

    IF::Instance().Print(640, 400, "01234 56789");
    IF::Instance().Print(640, 450, "01234 56789", 0x00FFFF);

    IF::Instance().Print(840, 200,
                         "01234 56789",
                         0xFF0000,
                         IF::TextAlignment::LEFT,
                         2.0);

    IF::Instance().Print(840, 220,
                         "01234 56789",
                         0x00FF00,
                         IF::TextAlignment::RIGHT,
                         2.0);

    IF::Instance().Print(840, 240,
                         "01234 56789",
                         0x0000FF,
                         IF::TextAlignment::CENTER,
                         2.0);

    IF::Instance().Printf(640, 500,
                          IF::TextParams::Set(),
                          "MEANING OF LIFE = %d", 42);

    IF::Instance().Printf(640, 500,
                          IF::TextParams::Set(0xFFFFFF,
                                              IF::TextAlignment::LEFT,
                                              4.0),
                          "SCALED");

    SDL_RenderPresent(r);
  }

  SDL_Log("Goodbye!");

  SDL_Quit();

  return 0;
}
