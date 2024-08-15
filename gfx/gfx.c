#include "gfx.h"

#include <ctype.h>
#include <string.h>

#include "../resources/symbols_res.h"
#include "esp_log.h"

// check is bit set, starting from most significant bit
// 8-bit version
#define GFX_IS_BIT_SET8(source, position) (source & (0x80 >> position))

ILI9341_t *_displayDevice = NULL;
Font_t *_activeFont = NULL;

void GFXInit(ILI9341_t *dev) {
  _displayDevice = dev;
  SymbolsResInit();
}

void GFXSetFont(Font_t *font) { _activeFont = font; }

void GFXDrawSymbol(SymbolData_t *symbol, _u16 xPos, _u16 yPos) {
  if (_displayDevice == NULL) return;
  if (_activeFont == NULL) return;

  if (_activeFont->scale == 0) {
    _activeFont->scale = 1;
  }

  _u8 symbolWidth = _activeFont->width;
  _u8 symbolHeight = _activeFont->height;
  _u8 scale = _activeFont->scale;
  _u16 color = _activeFont->color;

  _u16 scaledX = 0;
  _u16 scaledY = 0;
  _u8 line = 0;

  for (_u8 y = 0; y < symbolHeight; y++) {
    line = symbol->data[y];

    // if blank line - skip current step
    if (line == 0) continue;

    for (_u8 x = 0; x < symbolWidth; x++) {
      if (GFX_IS_BIT_SET8(line, x)) {
        scaledX = xPos + x * scale;
        scaledY = yPos + y * scale;
        Ili9341DrawPixels(_displayDevice, scaledX, scaledX + scale - 1, scaledY,
                          scaledY + scale - 1, color);
      }
    }
  }
}

void GFXDrawChar(_u8 asciiSymbol, _u16 xPos, _u16 yPos) {
  SymbolData_t *symbol = SymbolsGet(asciiSymbol);
  if (symbol == NULL) return;

  GFXDrawSymbol(symbol, xPos, yPos);
}

void GFXDrawString(const char *string, _u16 xPos, _u16 yPos) {
  _u16 x = xPos;
  _u16 length = strlen(string);
  _u8 letterInUpperCase;

  //_activeFont->height is taken because for font smaller than 8x8
  // height will contain actual size, when width is still 8 pt
  // for example, for 5x5 font its 8x5 (width x height).
  // Its known issue which will be fixed later
  _u8 letterWidth = (_activeFont->height) * (_activeFont->scale);
  _u8 spacing = _activeFont->scale;

  for (_u16 index = 0; index < length; index++) {
    letterInUpperCase = toupper(string[index]);
    GFXDrawChar(letterInUpperCase, x, yPos);
    x += letterWidth + spacing;
    if (x >= _displayDevice->width) {
      // TODO: add logic for word/letter wrap, ellipsize, etc.
      return;
    }
  }
}

void GFXDrawFilledRect(const _u16 left, const _u16 right, const _u16 top,
                       const _u16 bottom, const _u16 color) {
  Ili9341DrawPixels(_displayDevice, left, right, top, bottom, color);
}

void GFXFillScreen(const _u16 color) {
  if (_displayDevice == NULL) return;

  Ili9341DrawPixels(_displayDevice, 0, _displayDevice->width - 1, 0,
                    _displayDevice->height - 1, color);
}