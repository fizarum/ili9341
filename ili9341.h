#pragma once

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
#define SLPIN 0x10     // Sleep In
#define SLPOUT 0x11    // Sleep Out
#define DISPOFF 0x28   // Display OFF
#define DISPON 0x29    // Display ON
#define VSCRDEF 0x33   // Vertical Scrolling Definition
#define VSCRSADD 0x37  // Vertical Scrolling Start Address
#define POWER_CTRL1 0xC0
#define POWER_CTRL2 0xC1
#define VCOM_CTRL1 0xC5
#define VCOM_CTRL2 0xC7

// Backlight Control 7(PWM_OUT output frequency control.)
#define BCKL_CTRL7 0xBE

typedef enum {
  ANGLE_0,
  ANGLE_90,
  ANGLE_180,
  ANGLE_270,
} Rotation_t;

typedef enum {
  MODE_RGB = 0x00,
  MODE_BGR = 0x08,
} ColorMode_t;

typedef struct {
  _u16 width;
  _u16 height;

  /**
   * @brief scren rotation, counting from default (0)
   */
  Rotation_t rotation;

  /**
   * @brief scren rotation, counting from default (0)
   */
  Rotation_t font_rotaion;

  /**
   * @brief can RGB or BGR.
   * Check datasheet for details: "8.2.29 Memory Access Control (36h)" page 127
   */
  ColorMode_t color_mode;

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
  bool (*transmit_command)(const _u8 command);

  /** @brief Basic spi data transmit */
  bool (*transmit_data)(const _u8* data, const size_t length);

  bool (*lighten)(const _u8 percents);
} spi_display_t;

void display_init(spi_display_t* dev);

bool display_set_on_off(spi_display_t* dev, bool on);
bool display_sleep(spi_display_t* dev);
bool display_wakeup(spi_display_t* dev);

bool display_rotate(spi_display_t* dev, const Rotation_t rotation);
bool display_set_inversion(spi_display_t* dev, const bool inversion);
bool display_set_color_mode(spi_display_t* dev, const ColorMode_t mode);
void display_set_scroll_area(spi_display_t* dev, _u16 tfa, _u16 vsa, _u16 bfa);
void display_reset_scroll_area(spi_display_t* dev, _u16 vsa);
void display_scroll(spi_display_t* dev, _u16 vsp);

void display_draw_pixel(spi_display_t* dev, _u16 left, _u16 top, _u16 color);
void display_draw_pixels(spi_display_t* dev, _u16 left, _u16 top, _u16 right,
                         _u16 bottom, _u16* colors, size_t size);
#ifdef __cplusplus
}
#endif