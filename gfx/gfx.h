#ifndef GFX_H
#define GFX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../font/font.h"
#include "../ili9341.h"

void GFXInit(ILI9341_t *dev);

// symbols drawing
void GFXSetFont(Font_t *font);
void GFXDrawSymbol(SymbolData_t *symbol, _u16 xPos, _u16 yPos);
void GFXDrawChar(_u8 asciiSymbol, _u16 xPos, _u16 yPos);
void GFXDrawString(const char *string, _u16 xPos, _u16 yPos);

// objects drawing
void GFXDrawFilledRect(const _u16 left, const _u16 right, const _u16 top,
                       const _u16 bottom, const _u16 color);
//..

// utils
void GFXFillScreen(const _u16 color);

#ifdef __cplusplus
}
#endif
#endif  // GFX_H