#ifndef INSTANT_FONT_H
#define INSTANT_FONT_H

#include <SDL2/SDL.h>

#include <vector>
#include <string>
#include <cstring>

namespace IF
{
  class Font
  {
    public:

      // -----------------------------------------------------------------------

      ~Font()
      {
        SDL_DestroyTexture(_font);
      }

      // -----------------------------------------------------------------------

      void Init(SDL_Renderer* renderer)
      {
        if (_initialized)
        {
          SDL_Log("Font manager already initialized!");
          return;
        }

        _atlasWidth  = 16 * _fontWidth;
        _atlasHeight = 6 * _fontheight;

        _rendererRef = renderer;
        _font = SDL_CreateTexture(_rendererRef,
                                  SDL_PIXELFORMAT_RGBA32,
                                  SDL_TEXTUREACCESS_STREAMING,
                                  _atlasWidth, _atlasHeight);
        if (_font == nullptr)
        {
          SDL_Log("%s", SDL_GetError());
          return;
        }

        uint8_t* bytes = nullptr;
        int pitch = 0;
        int res = SDL_LockTexture(_font, nullptr, (void**)(&bytes), &pitch);
        if (res < 0)
        {
          SDL_Log("%s", SDL_GetError());
          return;
        }

        auto PutPixel = [this, bytes](size_t x,
                                      size_t y,
                                      const uint8_t (&pixel)[4])
        {
          std::memcpy(&bytes[ (x * _atlasWidth + y) * sizeof(pixel) ],
                      pixel,
                      sizeof(pixel));
        };

        for (size_t x = 0; x < _atlasHeight; x++)
        {
          for (size_t y = 0; y < _atlasWidth; y++)
          {
            PutPixel(x, y, kColorWhite);
          }
        }
        SDL_UnlockTexture(_font);

        _initialized = true;
      }

      // -----------------------------------------------------------------------

      SDL_Texture* GetFontTexture()
      {
        return _font;
      }

    private:
      bool _initialized = false;

      const uint8_t kColorWhite[4] = { 255, 255, 255, 255 };
      const uint8_t kColorBlack[4] = {   0,   0,   0, 255 };

      const uint8_t _fontWidth  = 10;
      const uint8_t _fontheight = 10;

      uint16_t _atlasWidth = 0;
      uint16_t _atlasHeight = 0;

      SDL_Texture*  _font        = nullptr;
      SDL_Renderer* _rendererRef = nullptr;

      //  !"#$%&'()*+,-./
      // 0123456789:;<=>?
      // @ABCDEFGHIJKLMNO
      // PQRSTUVWXYZ[\]^_
      // `abcdefghijklmno
      // pqrstuvwxyz{|}~

      //
      // TODO: replace with vector of uint16_t for speed amd memory (maybe).
      //
      const std::vector<std::vector<std::string>> _charMap =
      {
        // space
        {
          "..........",
          "..........",
          "..........",
          "..........",
          "..........",
          "..........",
          "..........",
          "..........",
          "..........",
          "..........",
        },
        // !
        {
          "..........",
          "....##....",
          "...####...",
          "...####...",
          "...####...",
          "....##....",
          "..........",
          "....##....",
          "....##....",
          ".........."
        },
        // "
        {
          "..........",
          "..........",
          "..##..##..",
          "..##..##..",
          "...#..#...",
          "..........",
          "..........",
          "..........",
          "..........",
          ".........."
        },
        // #
        {
          "..........",
          "..##..##..",
          "..##..##..",
          ".########.",
          "..##..##..",
          "..##..##..",
          ".########.",
          "..##..##..",
          "..##..##..",
          ".........."
        },
        // non-printable
        {
          "##########",
          "##......##",
          "#.#....#.#",
          "#..#..#..#",
          "#...##...#",
          "#...##...#",
          "#..#..#..#",
          "#.#....#.#",
          "##......##",
          "##########"
        }
      };
  };
}

#endif
