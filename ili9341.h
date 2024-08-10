#ifndef ILI9341_H
#define ILI9341_H

#ifdef __cplusplus
extern "C" {
#endif

#include "custom_hal_spi.h"

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
  int16_t dc;

  spi_device_handle_t handle;
} ILI9341_t;

void Ili9341Init(ILI9341_t *dev, _i8 res);

esp_err_t Ili9341PowerOn(ILI9341_t *dev, bool on);

esp_err_t Ili9341Rotate(ILI9341_t *dev, const Rotation_t rotation);
esp_err_t Ili9341SetInversion(ILI9341_t *dev, const bool inversionOn);
esp_err_t Ili9341SetColorMode(ILI9341_t *dev, const ColorMode_t mode);
void Ili9341SetScrollArea(ILI9341_t *dev, _u16 tfa, _u16 vsa, _u16 bfa);
void Ili9341ResetScrollArea(ILI9341_t *dev, _u16 vsa);
void Ili9341Scroll(ILI9341_t *dev, _u16 vsp);

// draw API
void Ili9341DrawPixel(ILI9341_t *dev, _u16 x, _u16 y, _u16 color);
void Ili9341DrawPixels(ILI9341_t *dev, _u16 left, _u16 right, _u16 top,
                       _u16 bottom, _u16 color);
void Ili9341FillScreen(ILI9341_t *dev, _u16 color);

#ifdef __cplusplus
}
#endif

#endif  // ILI9341_H
