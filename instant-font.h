#ifndef INSTANT_FONT_H
#define INSTANT_FONT_H

#include <SDL2/SDL.h>

#include <vector>
#include <string>
#include <cstring>

class IF
{
  public:

    // -----------------------------------------------------------------------

    ~IF()
    {
      SDL_DestroyTexture(_font);
    }

    // -----------------------------------------------------------------------

    static IF& Instance()
    {
      static IF instance;
      return instance;
    };

    // -----------------------------------------------------------------------

    void Init(SDL_Renderer* renderer)
    {
      if (_initialized)
      {
        SDL_Log("Font manager already initialized!");
        return;
      }

      _atlasWidth  = 16 * _fontWidth;
      _atlasHeight = 6 * _fontHeight;

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

      size_t charsDefined = _charMap.size();

      size_t charInd = 0;

      for (size_t x = 0; x < _atlasHeight; x += _fontHeight)
      {
        size_t xx = x;

        for (size_t y = 0; y < _atlasWidth; y += _fontWidth)
        {
          if (charInd < charsDefined)
          {
            size_t yy = y;

            const auto& glyph = _charMap[charInd];

            for (auto& line : glyph)
            {
              for (char c : line)
              {
                PutPixel(xx, yy, (c == '.') ? kColorBlack : kColorWhite);
                yy++;
              }

              yy = y;
              xx++;
            }

            xx = x;
          }

          charInd++;
        }
      }

      SDL_UnlockTexture(_font);

      _initialized = true;
    }

    // -------------------------------------------------------------------------

    //
    // FIXME: debug
    //
    void DumpTexture()
    {
      SDL_RenderCopy(_rendererRef, _font, nullptr, nullptr);
    }

  private:
    IF() = default;

    bool _initialized = false;

    const uint8_t kColorWhite[4] = { 255, 255, 255, 255 };
    const uint8_t kColorBlack[4] = {   0,   0,   0, 255 };

    const uint8_t _fontWidth  = 10;
    const uint8_t _fontHeight = 10;

    uint16_t _atlasWidth  = 0;
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

    using GlyphData = std::vector<std::vector<std::string>>;

    const GlyphData _charMap =
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
        ".##....##.",
        ".##....##.",
        "..#....#..",
        "..........",
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
      // $
      {
        "....##....",
        "..######..",
        ".#..##..#.",
        ".#..##....",
        "..######..",
        "....##..#.",
        ".#..##..#.",
        "..######..",
        "....##....",
        ".........."
      },
      // %
      {
        "..........",
        "..........",
        "..##...#..",
        "..##..#...",
        ".....#....",
        "....#.....",
        "...#..##..",
        "..#...##..",
        "..........",
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

#endif
