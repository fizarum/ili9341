#ifndef FONT_H
#define FONT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef uint8_t _u8;
typedef uint16_t _u16;

/**
 * @brief General font description, should be used
 * with SymbolData_t to define font size
 */
typedef struct Font_t {
  _u8 width;
  _u8 height;
  _u8 scale;
  _u16 color;
} Font_t;

/**
 * @brief Container for symbol bitmap
 */
typedef struct SymbolData_t {
  _u8 *data;
  _u16 dataLength;
  _u8 asciiCode;
} SymbolData_t;

#ifdef __cplusplus
}
#endif

#endif  // FONT_H
