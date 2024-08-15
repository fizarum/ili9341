#include "ili9341.h"

#include <driver/gpio.h>

#include "freertos/task.h"

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

void Ili9341SelectRegion(_i8 dc, spi_device_handle_t handle, _u16 l, _u16 r,
                         _u16 t, _u16 b);

void Ili9341Init(ILI9341_t *dev, _i8 res) {
  spi_device_handle_t handle = dev->handle;
  _u16 dc = dev->dc;

  PinSetAsOutput(dc, 0);
  PinSetAsOutput(res, 0);
  if (res >= 0) {
    // vTaskDelay(pdMS_TO_TICKS(20));
    vTaskDelay(pdMS_TO_TICKS(1));
    gpio_set_level(res, 1);
  }

  SPITransmitCommand(dc, handle, POWER_CTRL1);
  SPITransmitData(dc, handle, 0x23);

  SPITransmitCommand(dc, handle, POWER_CTRL2);
  SPITransmitData(dc, handle, 0x10);

  SPITransmitCommand(dc, handle, VCOM_CTRL1);
  SPITransmitData(dc, handle, 0x3E);
  SPITransmitData(dc, handle, 0x28);

  SPITransmitCommand(dc, handle, VCOM_CTRL2);
  SPITransmitData(dc, handle, 0x86);

  Ili9341Rotate(dev, Angle270);

  SPITransmitCommand(dc, handle, PIXSET);
  // 65K color: 16-bit/pixel
  SPITransmitData(dc, handle, 0x55);

  Ili9341SetInversion(dev, false);
  // lcdInversionOff(dev);

  SPITransmitCommand(dc, handle, FRMCTR1);
  SPITransmitData(dc, handle, 0x00);
  SPITransmitData(dc, handle, 0x18);

  SPITransmitCommand(dc, handle, DISCTRL);
  SPITransmitData(dc, handle, 0x08);
  // REV:1 GS:0 SS:0 SM:0
  SPITransmitData(dc, handle, 0xA2);
  SPITransmitData(dc, handle, 0x27);
  SPITransmitData(dc, handle, 0x00);

  SPITransmitCommand(dc, handle, GAMSET);
  SPITransmitData(dc, handle, 0x01);

  SPITransmitCommand(dc, handle, PGAMCTRL);
  SPITransmitData(dc, handle, 0x0F);
  SPITransmitData(dc, handle, 0x31);
  SPITransmitData(dc, handle, 0x2B);
  SPITransmitData(dc, handle, 0x0C);
  SPITransmitData(dc, handle, 0x0E);
  SPITransmitData(dc, handle, 0x08);
  SPITransmitData(dc, handle, 0x4E);
  SPITransmitData(dc, handle, 0xF1);
  SPITransmitData(dc, handle, 0x37);
  SPITransmitData(dc, handle, 0x07);
  SPITransmitData(dc, handle, 0x10);
  SPITransmitData(dc, handle, 0x03);
  SPITransmitData(dc, handle, 0x0E);
  SPITransmitData(dc, handle, 0x09);
  SPITransmitData(dc, handle, 0x00);

  SPITransmitCommand(dc, handle, NGAMCTRL);
  SPITransmitData(dc, handle, 0x00);
  SPITransmitData(dc, handle, 0x0E);
  SPITransmitData(dc, handle, 0x14);
  SPITransmitData(dc, handle, 0x03);
  SPITransmitData(dc, handle, 0x11);
  SPITransmitData(dc, handle, 0x07);
  SPITransmitData(dc, handle, 0x31);
  SPITransmitData(dc, handle, 0xC1);
  SPITransmitData(dc, handle, 0x48);
  SPITransmitData(dc, handle, 0x08);
  SPITransmitData(dc, handle, 0x0F);
  SPITransmitData(dc, handle, 0x0C);
  SPITransmitData(dc, handle, 0x31);
  SPITransmitData(dc, handle, 0x36);
  SPITransmitData(dc, handle, 0x0F);

  SPITransmitCommand(dc, handle, SLPOUT);
  /**
   * During the Resetting period, the display will be blanked
   * (The display is entering blanking sequence, which maximum time
   * is 120 ms, when Reset Starts in Sleep Out –mode.
   *
   * Check datasheet, page 218, "15.4. Reset Timing"
   */
  vTaskDelay(pdMS_TO_TICKS(121));
}

esp_err_t Ili9341PowerOn(ILI9341_t *dev, bool on) {
  return SPITransmitCommand(dev->dc, dev->handle,
                            on == true ? DISPON : DISPOFF);
}

esp_err_t Ili9341Rotate(ILI9341_t *dev, const Rotation_t rotation) {
  if (dev->rotation == rotation) {
    return ESP_OK;
  }

  _u8 mx, my, mv = 0x00;

  bool isOldModePortrait =
      (dev->rotation == Angle0) || (dev->rotation == Angle180);
  bool isNewModePortait = (rotation == Angle0) || (rotation == Angle180);

  SPITransmitCommand(dev->dc, dev->handle, MADCTL);

  switch (rotation) {
    case Angle90: {
      mx = 0x40;
      mv = 0x20;
      my = 0x00;
      break;
    }

    case Angle180: {
      mx = 0x40;
      my = 0x80;
      mv = 0x00;
      break;
    }

    case Angle270: {
      my = 0x80;
      mv = 0x20;
      mx = 0x00;
      break;
    }

    default:
      my = 0x00;
      mv = 0x00;
      mx = 0x00;
      break;
  }

  // update coords if orientaion hs been changed
  if (isOldModePortrait != isNewModePortait) {
    _u16 temp = dev->width;
    dev->width = dev->height;
    dev->height = temp;

    temp = dev->offsetx;
    dev->offsetx = dev->offsety;
    dev->offsety = temp;
  }
  dev->rotation = rotation;
  return SPITransmitData(dev->dc, dev->handle, mx | my | mv | (dev->colorMode));
}

esp_err_t Ili9341SetInversion(ILI9341_t *dev, const bool inversionOn) {
  return SPITransmitCommand(dev->dc, dev->handle,
                            inversionOn == true ? DINVON : DINVOFF);
}

esp_err_t Ili9341SetColorMode(ILI9341_t *dev, const ColorMode_t mode) {
  dev->colorMode = mode;
  SPITransmitCommand(dev->dc, dev->handle, MADCTL);
  return SPITransmitData(dev->dc, dev->handle, mode);
}

/**
 * @brief Set area as scrollable
 *
 * @param tfa Top Fixed Area (in No. of lines from Top of
 * the Frame Memory and Display)
 * @param vsa Vertical Scrolling Area  (in No. of lines of the Frame
 * Memory [not the display] from the Vertical Scrolling Start Address).
 * The first line read from Frame Memory appears immediately after
 * the bottom most line of the Top Fixed Area.
 * @param bfa Bottom Fixed Area (in No. of lines from Bottom of the Frame
 * Memory and Display). TFA, VSA and BFA refer to the Frame Memory Line Pointer.
 */
void Ili9341SetScrollArea(ILI9341_t *dev, _u16 tfa, _u16 vsa, _u16 bfa) {
  SPITransmitCommand(dev->dc, dev->handle, VSCRDEF);
  SPITransmitDataWord(dev->dc, dev->handle, tfa);
  SPITransmitDataWord(dev->dc, dev->handle, vsa);
  SPITransmitDataWord(dev->dc, dev->handle, bfa);
}

void Ili9341ResetScrollArea(ILI9341_t *dev, _u16 vsa) {
  SPITransmitCommand(dev->dc, dev->handle, VSCRDEF);
  SPITransmitDataWord(dev->dc, dev->handle, 0);
  SPITransmitDataWord(dev->dc, dev->handle, vsa);
  SPITransmitDataWord(dev->dc, dev->handle, 0);
}

/**
 * @brief Vertical Scrolling Start Address
 *
 * @param vsp Vertical Scrolling Start Address, the line in the
 * Frame Memory that will be written as the first line after
 * the last line of the Top Fixed Area on the display
 */
void Ili9341Scroll(ILI9341_t *dev, _u16 vsp) {
  SPITransmitCommand(dev->dc, dev->handle, VSCRSADD);
  SPITransmitDataWord(dev->dc, dev->handle, vsp);
}

void Ili9341DrawPixel(ILI9341_t *dev, _u16 x, _u16 y, _u16 color) {
  Ili9341DrawPixels(dev, x, x, y, y, color);
}

/**
 * @brief Fill display region by provided color
 */
void Ili9341DrawPixels(ILI9341_t *dev, _u16 left, _u16 right, _u16 top,
                       _u16 bottom, _u16 color) {
  left += dev->offsetx;
  right += dev->offsetx;
  top += dev->offsety;
  bottom += dev->offsety;

  if (left >= dev->width) return;
  if (top >= dev->height) return;

  if (right >= dev->width) {
    right = dev->width - 1;
  }

  if (bottom >= dev->height) {
    bottom = dev->height - 1;
  }

  Ili9341SelectRegion(dev->dc, dev->handle, left, right, top, bottom);
  SPITransmitCommand(dev->dc, dev->handle, RAMWR);

  // case of drawing one pixel - optimized part
  if (left == right && top == bottom) {
    SPITransmitDataWord(dev->dc, dev->handle, color);
    return;
  }

  _u16 pixelsCountToFill = bottom - top + 1;

  for (_u16 x = left; x <= right; x++) {
    SPITransmitDataTimes(dev->dc, dev->handle, color, pixelsCountToFill);
  }
}

void Ili9341SelectRegion(_i8 dc, spi_device_handle_t handle, _u16 l, _u16 r,
                         _u16 t, _u16 b) {
  SPITransmitCommand(dc, handle, CASET);
  SPITransmitDataDWord(dc, handle, l, r);

  SPITransmitCommand(dc, handle, PASET);
  SPITransmitDataDWord(dc, handle, t, b);
}