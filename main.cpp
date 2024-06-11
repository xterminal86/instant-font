#include "instant-font.h"

bool IsRunning = true;

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

int main()
{
  if (SDL_Init(SDL_INIT_VIDEO) != 0)
  {
    printf("SDL_Init Error: %s\n", SDL_GetError());
    return 1;
  }

  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);

  SDL_Window* window = SDL_CreateWindow("Printer test",
                                        0, 0,
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

  IF::Font font;

  font.Init(r);

  SDL_SetRenderDrawColor(r, 0, 0, 0, 255);

  SDL_Event evt;

  while (IsRunning)
  {
    while (SDL_PollEvent(&evt))
    {
      HandleEvent(evt);
    }

    SDL_RenderClear(r);

    SDL_RenderCopy(r, font.GetFontTexture(), nullptr, nullptr);

    SDL_RenderPresent(r);
  }

  SDL_Log("Goodbye!");

  SDL_Quit();

  return 0;
}
