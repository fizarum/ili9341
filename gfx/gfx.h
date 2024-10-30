#ifndef GFX_H
#define GFX_H

#ifdef __cplusplus
extern "C" {
#endif

#include <types.h>

#include "../font/font.h"

typedef void (*GFX_Callback)(const _u16 left, const _u16 top, const _u16 right,
                             const _u16 bottom, _u16 color);

void GFXInit(const _u16 width, const _u16 height, GFX_Callback callback);

// symbols drawing
void GFXSetFont(Font_t *font);
void GFX_SetBackground(_u16 color);

void GFXDrawSymbol(SymbolData_t *symbol, _u16 xPos, _u16 yPos);
void GFXDrawChar(_u8 asciiSymbol, _u16 xPos, _u16 yPos);
void GFXDrawString(const char *string, _u16 xPos, _u16 yPos);

// objects drawing
void GFXDrawFilledRect(const _u16 left, const _u16 right, const _u16 top,
                       const _u16 bottom, const _u16 color);
// utils
void GFXFillScreen(const _u16 color);

_u16 GFXGetFontColor();
_u16 GFX_GetBackgroundColor();

#ifdef __cplusplus
}
#endif
#endif  // GFX_H