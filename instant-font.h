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
      SDL_DestroyTexture(_fontAtlas);
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

      _atlasWidth  = _numTilesH * _fontSize;
      _atlasHeight = _numTilesV * _fontSize;

      _rendererRef = renderer;
      _fontAtlas = SDL_CreateTexture(_rendererRef,
                                SDL_PIXELFORMAT_RGBA32,
                                SDL_TEXTUREACCESS_STREAMING,
                                _atlasWidth, _atlasHeight);
      if (_fontAtlas == nullptr)
      {
        SDL_Log("%s", SDL_GetError());
        return;
      }

      uint8_t* bytes = nullptr;
      int pitch = 0;
      int res = SDL_LockTexture(_fontAtlas, nullptr, (void**)(&bytes), &pitch);
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

      for (size_t x = 0; x < _atlasHeight; x += _fontSize)
      {
        size_t xx = x;

        for (size_t y = 0; y < _atlasWidth; y += _fontSize)
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

      SDL_UnlockTexture(_fontAtlas);

      _initialized = true;
    }

    // -------------------------------------------------------------------------

    enum class TextAlignment
    {
      LEFT = 0,
      CENTER,
      RIGHT
    };

    // -------------------------------------------------------------------------

    struct TextParams
    {
      TextAlignment Align = TextAlignment::LEFT;
      uint32_t      Color = 0xFFFFFF;
      size_t        Scale = 1;

      static TextParams Default()
      {
        static TextParams params;
        return params;
      }

      static TextParams Scaled(size_t factor)
      {
        static TextParams params;
        params.Scale = factor;
        return params;
      }
    };

    // -------------------------------------------------------------------------

    void Print(int x, int y,
               const std::string& text,
               uint32_t color = 0xFFFFFF,
               TextAlignment align = TextAlignment::LEFT,
               size_t scaleFactor = 1)
    {
      SaveColor();

      SDL_SetRenderDrawBlendMode(_rendererRef,
                                 ( (color & _maskA) != 0 )
                                 ? SDL_BLENDMODE_BLEND
                                 : SDL_BLENDMODE_NONE);

      const auto& clr = HTML2RGBA(color);

      SDL_SetTextureColorMod(_fontAtlas, clr.r, clr.g, clr.b);

      size_t ln = text.length();

      size_t xOffset = 0;

      switch (align)
      {
        // --------------------------
        case TextAlignment::LEFT:
          xOffset = 0;
          break;
        // --------------------------
        case TextAlignment::RIGHT:
          xOffset = ln;
          break;
        // --------------------------
        case TextAlignment::CENTER:
          xOffset = -(ln / 2);
          break;
        // --------------------------
        default:
          break;
        // --------------------------
      }

      //
      // Draw text.
      //
      for (char c : text)
      {
        uint8_t charInd = c - 32;

        if (c < 32 or c > 127)
        {
          charInd = _charMap.size() - 1;
        }

        size_t xx = (charInd % _numTilesH);
        size_t yy = (charInd / _numTilesH);

        static SDL_Rect fromAtlas;

        fromAtlas.x = xx * _fontSize;
        fromAtlas.y = yy * _fontSize;

        fromAtlas.w = _fontSize;
        fromAtlas.h = _fontSize;

        static SDL_Rect dst;
        dst.x = x + (xOffset * _fontSize * scaleFactor);
        dst.y = y;
        dst.w = _fontSize * scaleFactor;
        dst.h = _fontSize * scaleFactor;

        SDL_RenderCopy(_rendererRef, _fontAtlas, &fromAtlas, &dst);

        x += (_fontSize * scaleFactor);
      }

      RestoreColor();
    }

    // -------------------------------------------------------------------------

    template <typename ... Args>
    void Printf(int x, int y,
                TextParams params,
                const std::string& formatString,
                Args ... args)
    {
      static size_t size = ::snprintf(nullptr,
                                      0,
                                      formatString.data(),
                                      args ...);
      if (size == 0)
      {
        return;
      }

      static std::string s;
      s.resize(size);

      char* buf = (char*)s.data();
      ::snprintf(buf, size + 1, formatString.data(), args ...);

      Print(x, y, s, params.Color, params.Align, params.Scale);
    }

    // -------------------------------------------------------------------------

    //
    // FIXME: debug
    //
    void DumpTexture()
    {
      SDL_Rect dst;
      dst.x = 0;
      dst.y = 0;
      dst.w = _atlasWidth * 5;
      dst.h = _atlasHeight * 5;

      SDL_RenderCopy(_rendererRef, _fontAtlas, nullptr, &dst);
    }

    // -------------------------------------------------------------------------

  private:
    IF() = default;

    // -------------------------------------------------------------------------

    const SDL_Color& HTML2RGBA(const uint32_t& colorMask)
    {
      if (colorMask <= 0xFFFFFF)
      {
        _drawColor.r = (colorMask & _maskR) >> 16;
        _drawColor.g = (colorMask & _maskG) >> 8;
        _drawColor.b = (colorMask & _maskB);
        _drawColor.a = 0xFF;
      }
      else
      {
        _drawColor.a = (colorMask & _maskA) >> 24;
        _drawColor.r = (colorMask & _maskR) >> 16;
        _drawColor.g = (colorMask & _maskG) >> 8;
        _drawColor.b = (colorMask & _maskB);
      }

      return _drawColor;
    }

    // -------------------------------------------------------------------------

    void SaveColor()
    {
      SDL_GetTextureColorMod(_fontAtlas,
                             &_oldColor.r,
                             &_oldColor.g,
                             &_oldColor.b);
    }

    // -------------------------------------------------------------------------

    void RestoreColor()
    {
      SDL_SetTextureColorMod(_fontAtlas,
                             _oldColor.r,
                             _oldColor.g,
                             _oldColor.b);
    }

    const uint32_t _maskR = 0x00FF0000;
    const uint32_t _maskG = 0x0000FF00;
    const uint32_t _maskB = 0x000000FF;
    const uint32_t _maskA = 0xFF000000;

    SDL_Color _drawColor;
    SDL_Color _oldColor;

    bool _initialized = false;

    const uint8_t kColorWhite[4] = { 255, 255, 255, 255 };
    const uint8_t kColorBlack[4] = {   0,   0,   0, 255 };

    const uint8_t _numTilesH = 16;
    const uint8_t _numTilesV = 6;

    const uint8_t _fontSize = 9;

    uint16_t _atlasWidth  = 0;
    uint16_t _atlasHeight = 0;

    SDL_Texture*  _fontAtlas   = nullptr;
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
        ".........",
        ".........",
        ".........",
        ".........",
        ".........",
        ".........",
        ".........",
        ".........",
        ".........",
      },
      // !
      {
        ".........",
        "....#....",
        "...###...",
        "...###...",
        "...###...",
        "....#....",
        ".........",
        "....#....",
        "........."
      },
      // "
      {
        ".........",
        ".##...##.",
        ".##...##.",
        "..#...#..",
        ".........",
        ".........",
        ".........",
        ".........",
        "........."
      },
      // #
      {
        ".........",
        ".##..##..",
        ".##..##..",
        "########.",
        ".##..##..",
        "########.",
        ".##..##..",
        ".##..##..",
        "........."
      },
      // $
      {
        "....#....",
        "..#####..",
        ".#..#..#.",
        ".#..#....",
        "..#####..",
        "....#..#.",
        ".#..#..#.",
        "..#####..",
        "....#...."
      },
      // %
      {
        ".........",
        ".........",
        ".##...#..",
        ".##..#...",
        "....#....",
        "...#.....",
        "..#..##..",
        ".#...##..",
        "........."
      },
      // &
      {
        "..##.....",
        ".#..#....",
        ".#..#....",
        "..##...#.",
        ".#..#.#..",
        "#....#...",
        "#...#.#..",
        ".###...#.",
        "........."
      },
      // '
      {
        ".........",
        "....##...",
        "....##...",
        "....#....",
        "...#.....",
        ".........",
        ".........",
        ".........",
        ".........",
      },
      // (
      {
        ".........",
        "....###..",
        "...##....",
        "..##.....",
        "..##.....",
        "..##.....",
        "...##....",
        "....###..",
        ".........",
      },
      // )
      {
        ".........",
        "..###....",
        "....##...",
        ".....##..",
        ".....##..",
        ".....##..",
        "....##...",
        "..###....",
        ".........",
      },
      // *
      {
        ".........",
        ".#..#..#.",
        "..#.#.#..",
        "...###...",
        ".#######.",
        "...###...",
        "..#.#.#..",
        ".#..#..#.",
        ".........",
      },
      // +
      {
        ".........",
        ".........",
        "....#....",
        "....#....",
        "..#####..",
        "....#....",
        "....#....",
        ".........",
        ".........",
      },
      // ,
      {
        ".........",
        ".........",
        ".........",
        ".........",
        "....##...",
        "....##...",
        "....#....",
        "...#.....",
        ".........",
      },
      // -
      {
        ".........",
        ".........",
        ".........",
        ".........",
        ".#######.",
        ".........",
        ".........",
        ".........",
        ".........",
      },
      // .
      {
        ".........",
        ".........",
        ".........",
        ".........",
        ".........",
        ".........",
        "....##...",
        "....##...",
        ".........",
      },
      // /
      {
        ".........",
        ".......#.",
        "......#..",
        ".....#...",
        "....#....",
        "...#.....",
        "..#......",
        ".#.......",
        ".........",
      },
      // 0
      {
        ".........",
        "...###...",
        "..#...#..",
        ".##...##.",
        ".##.#.##.",
        ".##...##.",
        "..#...#..",
        "...###...",
        ".........",
      },
      // 1
      {
        ".........",
        "....##...",
        "...###...",
        "..####...",
        "....##...",
        "....##...",
        "....##...",
        "..######.",
        ".........",
      },
      // 2
      {
        ".........",
        "..#####..",
        ".##...##.",
        ".....##..",
        "....##...",
        "...##....",
        "..##..##.",
        ".#######.",
        ".........",
      },
      // 3
      {
        ".........",
        "..#####..",
        ".##...##.",
        "......##.",
        "....###..",
        "......##.",
        ".##...##.",
        "..#####..",
        ".........",
      },
      // 4
      {
        ".........",
        "...####..",
        "..##.##..",
        ".##..##..",
        ".#######.",
        ".....##..",
        ".....##..",
        "....####.",
        ".........",
      },
      // 5
      {
        ".........",
        ".######..",
        ".##......",
        ".######..",
        "......##.",
        "......##.",
        ".##...##.",
        "..#####..",
        ".........",
      },
      // 6
      {
        ".........",
        "...####..",
        "..##.....",
        ".##......",
        ".######..",
        ".##...##.",
        ".##...##.",
        "..#####..",
        ".........",
      },
      // 7
      {
        ".........",
        ".#######.",
        ".##...##.",
        ".....##..",
        "....##...",
        "...##....",
        "...##....",
        "...##....",
        ".........",
      },
      // 8
      {
        ".........",
        "..#####..",
        ".##...##.",
        ".##...##.",
        "..#####..",
        ".##...##.",
        ".##...##.",
        "..#####..",
        ".........",
      },
      // 9
      {
        ".........",
        "..#####..",
        ".##...##.",
        ".##...##.",
        "..######.",
        "......##.",
        ".....##..",
        "..####...",
        ".........",
      },
      // :
      {
        ".........",
        ".........",
        "....##...",
        "....##...",
        ".........",
        ".........",
        "....##...",
        "....##...",
        ".........",
      },
      // ;
      {
        ".........",
        ".........",
        "....##...",
        "....##...",
        ".........",
        "....##...",
        "....##...",
        "...##....",
        ".........",
      },
      // <
      {
        ".........",
        "......##.",
        "....##...",
        "..##.....",
        "##.......",
        "..##.....",
        "....##...",
        "......##.",
        ".........",
      },
      // =
      {
        ".........",
        ".........",
        ".........",
        "..#####..",
        ".........",
        "..#####..",
        ".........",
        ".........",
        ".........",
      },
      // >
      {
        ".........",
        "##.......",
        "..##.....",
        "....##...",
        "......##.",
        "....##...",
        "..##.....",
        "##.......",
        ".........",
      },
      // ?
      {
        ".........",
        "...####..",
        "..##..##.",
        "..##..##.",
        ".....##..",
        "....##...",
        "....##...",
        ".........",
        "....##...",
      },
      // @
      {
        ".........",
        "..####...",
        ".#..###..",
        ".#.#..#..",
        ".#.#..#..",
        ".#..##...",
        ".#.....#.",
        "..#####..",
        ".........",
      },
      // A
      {
        ".........",
        "....#....",
        "...###...",
        "..##.##..",
        ".##...##.",
        ".#######.",
        ".##...##.",
        ".##...##.",
        ".........",
      },
      // B
      {
        ".........",
        ".######..",
        ".##...##.",
        ".##...##.",
        ".######..",
        ".##...##.",
        ".##...##.",
        ".######..",
        ".........",
      },
      // C
      {
        ".........",
        "..#####..",
        ".##...##.",
        ".##......",
        ".##......",
        ".##......",
        ".##...##.",
        "..#####..",
        ".........",
      },
      // D
      {
        ".........",
        ".######..",
        "..##..##.",
        "..##..##.",
        "..##..##.",
        "..##..##.",
        "..##..##.",
        ".######..",
        ".........",
      },
      // E
      {
        ".........",
        ".#######.",
        "..##..##.",
        "..##.....",
        "..####...",
        "..##.....",
        "..##..##.",
        ".#######.",
        ".........",
      },
      // F
      {
        ".........",
        ".#######.",
        "..##..##.",
        "..##.....",
        "..####...",
        "..##.....",
        "..##.....",
        ".#####...",
        ".........",
      },
      // G
      {
        ".........",
        "..#####..",
        ".##...##.",
        ".##......",
        ".##......",
        ".##..###.",
        ".##...##.",
        "..######.",
        ".........",
      },
      // H
      {
        ".........",
        ".##...##.",
        ".##...##.",
        ".##...##.",
        ".#######.",
        ".##...##.",
        ".##...##.",
        ".##...##.",
        ".........",
      },
      // I
      {
        ".........",
        "...####..",
        "....##...",
        "....##...",
        "....##...",
        "....##...",
        "....##...",
        "...####..",
        ".........",
      },
      // J
      {
        ".........",
        "....####.",
        ".....##..",
        ".....##..",
        ".....##..",
        ".##..##..",
        ".##..##..",
        "..####...",
        ".........",
      },
      // K
      {
        ".........",
        ".##..##..",
        ".##..##..",
        ".##.##...",
        ".####....",
        ".##.##...",
        ".##..##..",
        ".##..##..",
        ".........",
      },
      // L
      {
        ".........",
        ".####....",
        "..##.....",
        "..##.....",
        "..##.....",
        "..##.....",
        "..##..##.",
        ".#######.",
        ".........",
      },
      // M
      {
        ".........",
        ".##...##.",
        ".###.###.",
        ".#######.",
        ".##.#.##.",
        ".##...##.",
        ".##...##.",
        ".##...##.",
        ".........",
      },
      // N
      {
        ".........",
        ".##...##.",
        ".###..##.",
        ".####.##.",
        ".##.####.",
        ".##..###.",
        ".##...##.",
        ".##...##.",
        ".........",
      },
      // O
      {
        ".........",
        "..#####..",
        ".##...##.",
        ".##...##.",
        ".##...##.",
        ".##...##.",
        ".##...##.",
        "..#####..",
        ".........",
      },
      // non-printable
      {
        "#########",
        "##.....##",
        "#.#...#.#",
        "#..#.#..#",
        "#...#...#",
        "#..#.#..#",
        "#.#...#.#",
        "##.....##",
        "#########"
      }
    };
};

#endif
