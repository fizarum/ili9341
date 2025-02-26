#ifndef ILI9341_H
#define ILI9341_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint8_t _u8;
typedef int8_t _i8;
typedef uint16_t _u16;
typedef uint32_t _u32;

#define CASET 0x2A     // set column(x) address
#define PASET 0x2B     // set Page(y) address
#define RAMWR 0x2C     // Memory Write
#define MADCTL 0x36    // Memory Access Control
#define PIXSET 0x3A    // Pixel Format Set
#define DINVOFF 0x20   // Display Inversion OFF
#define DINVON 0x21    // Display Inversion
#define FRMCTR1 0xB1   // Frame Rate Control
#define DISCTRL 0xB6   // Display Function Control
#define GAMSET 0x26    // Gamma Set
#define PGAMCTRL 0xE0  // Positive Gamma Correction
#define NGAMCTRL 0xE1  // Negative Gamma Correction
#define SLPOUT 0x11    // Sleep Out
#define DISPOFF 0x28   // Display OFF
#define DISPON 0x29    // Display ON
#define VSCRDEF 0x33   // Vertical Scrolling Definition
#define VSCRSADD 0x37  // Vertical Scrolling Start Address
#define POWER_CTRL1 0xC0
#define POWER_CTRL2 0xC1
#define VCOM_CTRL1 0xC5
#define VCOM_CTRL2 0xC7

typedef enum {
  Angle0,
  Angle90,
  Angle180,
  Angle270,
} Rotation_t;

typedef enum {
  ModeRGB = 0x00,
  ModeBGR = 0x08,
} ColorMode_t;

typedef struct {
  _u16 width;
  _u16 height;
  _u16 offsetx;
  _u16 offsety;

  /**
   * @brief scren rotation, counting from default (0)
   */
  Rotation_t rotation;

  /**
   * @brief scren rotation, counting from default (0)
   */
  Rotation_t fontRotaion;

  /**
   * @brief can RGB or BGR.
   * Check datasheet for details: "8.2.29 Memory Access Control (36h)" page 127
   */
  ColorMode_t colorMode;

  /**
   * @brief D/C gpio pin
   */
  _i8 dc;

  /**
   * @brief Reset gpio pin
   */
  _i8 res;

  /**
   * @brief Backlight gpio pin
   */
  _i8 bl;

  /** @brief Basic spi command transmit */
  bool (*transmitCommand)(const _u8 command);

  /** @brief Basic spi data transmit */
  bool (*transmitData)(const _u8 *data, const size_t length);

  /** @brief transmit byte n times */
  bool (*transmitDataTimes)(const _u16 value, _u16 times);

  bool (*lighten)(const _u8 percents);
} ILI9341_t;

void Ili9341Init(ILI9341_t *dev);

bool Ili9341PowerOn(ILI9341_t *dev, bool on);

bool Ili9341Rotate(ILI9341_t *dev, const Rotation_t rotation);
bool Ili9341SetInversion(ILI9341_t *dev, const bool inversionOn);
bool Ili9341SetColorMode(ILI9341_t *dev, const ColorMode_t mode);
void Ili9341SetScrollArea(ILI9341_t *dev, _u16 tfa, _u16 vsa, _u16 bfa);
void Ili9341ResetScrollArea(ILI9341_t *dev, _u16 vsa);
void Ili9341Scroll(ILI9341_t *dev, _u16 vsp);

void Ili9341DrawPixel(ILI9341_t *dev, _u16 left, _u16 top, _u16 color);
void Ili9341DrawPixels(ILI9341_t *dev, _u16 left, _u16 top, _u16 right,
                       _u16 bottom, _u16 *colors, size_t colorsSize);
void Ili9341DrawPixelTimes(ILI9341_t *dev, _u16 left, _u16 top, _u16 right,
                           _u16 bottom, _u16 color);
#ifdef __cplusplus
}
#endif

#endif  // ILI9341_H
