#ifndef ILI9341_H
#define ILI9341_H

#ifdef __cplusplus
extern "C" {
#endif

#include "custom_hal_spi.h"

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

  _i8 res;

  spi_device_handle_t handle;
} ILI9341_t;

void Ili9341Init(ILI9341_t *dev);

esp_err_t Ili9341PowerOn(ILI9341_t *dev, bool on);

esp_err_t Ili9341Rotate(ILI9341_t *dev, const Rotation_t rotation);
esp_err_t Ili9341SetInversion(ILI9341_t *dev, const bool inversionOn);
esp_err_t Ili9341SetColorMode(ILI9341_t *dev, const ColorMode_t mode);
void Ili9341SetScrollArea(ILI9341_t *dev, _u16 tfa, _u16 vsa, _u16 bfa);
void Ili9341ResetScrollArea(ILI9341_t *dev, _u16 vsa);
void Ili9341Scroll(ILI9341_t *dev, _u16 vsp);

void Ili9341DrawPixel(ILI9341_t *dev, _u16 x, _u16 y, _u16 color);
void Ili9341DrawPixelTimes(ILI9341_t *dev, _u16 left, _u16 right, _u16 top,
                           _u16 bottom, _u16 color);

_u16 GFXGetFontColor();

#ifdef __cplusplus
}
#endif

#endif  // ILI9341_H
