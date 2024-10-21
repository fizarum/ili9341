#include "ili9341.h"

#include <driver/gpio.h>

#include "freertos/semphr.h"
#include "freertos/task.h"

static SemaphoreHandle_t mutex;

void Ili9341SelectRegion(_i8 dc, spi_device_handle_t handle, _u16 l, _u16 r,
                         _u16 t, _u16 b);

void Ili9341Init(ILI9341_t *dev) {
  mutex = xSemaphoreCreateMutex();
  spi_device_handle_t handle = dev->handle;
  _u16 dc = dev->dc;
  _i8 res = dev->res;

  PinSetAsOutput(dc, 0);
  PinSetAsOutput(res, 0);
  if (res >= 0) {
    // vTaskDelay(pdMS_TO_TICKS(20));
    vTaskDelay(pdMS_TO_TICKS(1));
    gpio_set_level(res, 1);
  }

  SPITransmitCommand(dc, handle, POWER_CTRL1);
  // SPITransmitData(dc, handle, 0x23);
  /**
  0x1F - 4.4V
  0x17 - 4.0V
  0x11 - 3.7V
  0x09 - 3.3V
  */
  SPITransmitData(dc, handle, 0x09);

  SPITransmitCommand(dc, handle, POWER_CTRL2);
  // BT[2:0] = 0x03, means VGH = VCI x 6, VGL = - VCI x 3
  // lowest value and default
  SPITransmitData(dc, handle, 0x03);

  SPITransmitCommand(dc, handle, VCOM_CTRL1);
  /**
  VMH: 0x1E = 3.45V
  0x3E = 5.850V
  VML: 0x28 = -1.500V
  */
  SPITransmitData(dc, handle, 0x1E);
  SPITransmitData(dc, handle, 0x28);

  SPITransmitCommand(dc, handle, VCOM_CTRL2);
  // VCOMH/VCOML voltage adjustment
  SPITransmitData(dc, handle, 0x86);

  // just give some time to apply some changes, when mcu has high
  // clock speed (240 MHz, for example) this part works incorrectly
  // without delay
  vTaskDelay(pdMS_TO_TICKS(10));

  Ili9341Rotate(dev, Angle270);

  SPITransmitCommand(dc, handle, PIXSET);
  // 65K color: 16-bit/pixel
  SPITransmitData(dc, handle, 0x55);

  Ili9341SetInversion(dev, false);
  // lcdInversionOff(dev);

  SPITransmitCommand(dc, handle, FRMCTR1);
  SPITransmitData(dc, handle, 0x00);
  /**
  Frame Rate:
  0x1B = 70 Hz
  0x19 = 76 Hz
  0x15 = 90 Hz
  0x13 = 100 Hz
  */
  SPITransmitData(dc, handle, 0x13);

  SPITransmitCommand(dc, handle, DISCTRL);
  /**
  PT = 0x02: Determine source/VCOM output in a non-display area in the partial
  display mode.

  PTG = 0x02: interval scan
  */
  SPITransmitData(dc, handle, 0x0A);
  /**
  REV:1 - liqud crystal normally white
  GS:0 SS:1 SM:0
  SS:1, means S720 -> S1 (because of rotated display)
  ISC = 0x01 - scan cycle 3 frames (51 ms)
  */
  SPITransmitData(dc, handle, 0xA1);
  // NL - 320 lines
  SPITransmitData(dc, handle, 0x27);
  SPITransmitData(dc, handle, 0x00);

  SPITransmitCommand(dc, handle, PGAMCTRL);
  _u8 pgc[] = {
      0x1F, 0x1A, 0x18, 0x0A, 0x0F, 0x06, 0x45, 0x87,
      0x32, 0x0A, 0x07, 0x02, 0x07, 0x05, 0x00,
  };

  SPITransmitDataArray(dc, handle, pgc, sizeof(pgc));

  SPITransmitCommand(dc, handle, NGAMCTRL);
  _u8 ngc[] = {
      0x00, 0x25, 0x27, 0x05, 0x10, 0x09, 0x3A, 0x78,
      0x4D, 0x05, 0x18, 0x0D, 0x38, 0x3A, 0x1F,
  };

  SPITransmitDataArray(dc, handle, ngc, sizeof(ngc));

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
  xSemaphoreTake(mutex, portMAX_DELAY);

  esp_err_t result =
      SPITransmitCommand(dev->dc, dev->handle, on == true ? DISPON : DISPOFF);

  xSemaphoreGive(mutex);
  return result;
}

esp_err_t Ili9341Rotate(ILI9341_t *dev, const Rotation_t rotation) {
  xSemaphoreTake(mutex, portMAX_DELAY);

  if (dev->rotation == rotation) {
    xSemaphoreGive(mutex);
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
  esp_err_t result =
      SPITransmitData(dev->dc, dev->handle, mx | my | mv | (dev->colorMode));

  xSemaphoreGive(mutex);

  return result;
}

esp_err_t Ili9341SetInversion(ILI9341_t *dev, const bool inversionOn) {
  xSemaphoreTake(mutex, portMAX_DELAY);

  esp_err_t result = SPITransmitCommand(dev->dc, dev->handle,
                                        inversionOn == true ? DINVON : DINVOFF);

  xSemaphoreGive(mutex);
  return result;
}

esp_err_t Ili9341SetColorMode(ILI9341_t *dev, const ColorMode_t mode) {
  xSemaphoreTake(mutex, portMAX_DELAY);

  dev->colorMode = mode;
  SPITransmitCommand(dev->dc, dev->handle, MADCTL);
  esp_err_t result = SPITransmitData(dev->dc, dev->handle, mode);

  xSemaphoreGive(mutex);
  return result;
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
  xSemaphoreTake(mutex, portMAX_DELAY);

  SPITransmitCommand(dev->dc, dev->handle, VSCRDEF);
  SPITransmitDataWord(dev->dc, dev->handle, tfa);
  SPITransmitDataWord(dev->dc, dev->handle, vsa);
  SPITransmitDataWord(dev->dc, dev->handle, bfa);

  xSemaphoreGive(mutex);
}

void Ili9341ResetScrollArea(ILI9341_t *dev, _u16 vsa) {
  xSemaphoreTake(mutex, portMAX_DELAY);

  SPITransmitCommand(dev->dc, dev->handle, VSCRDEF);
  SPITransmitDataWord(dev->dc, dev->handle, 0);
  SPITransmitDataWord(dev->dc, dev->handle, vsa);
  SPITransmitDataWord(dev->dc, dev->handle, 0);

  xSemaphoreGive(mutex);
}

/**
 * @brief Vertical Scrolling Start Address
 *
 * @param vsp Vertical Scrolling Start Address, the line in the
 * Frame Memory that will be written as the first line after
 * the last line of the Top Fixed Area on the display
 */
void Ili9341Scroll(ILI9341_t *dev, _u16 vsp) {
  xSemaphoreTake(mutex, portMAX_DELAY);

  SPITransmitCommand(dev->dc, dev->handle, VSCRSADD);
  SPITransmitDataWord(dev->dc, dev->handle, vsp);

  xSemaphoreGive(mutex);
}

void Ili9341DrawPixel(ILI9341_t *dev, _u16 x, _u16 y, _u16 color) {
  Ili9341DrawPixelTimes(dev, x, x, y, y, color);
}

/**
 * @brief Fill display region by provided color
 */
void Ili9341DrawPixelTimes(ILI9341_t *dev, _u16 left, _u16 right, _u16 top,
                           _u16 bottom, _u16 color) {
  xSemaphoreTake(mutex, portMAX_DELAY);
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

  int16_t dc = dev->dc;
  spi_device_handle_t handle = dev->handle;

  Ili9341SelectRegion(dc, handle, left, right, top, bottom);
  SPITransmitCommand(dc, handle, RAMWR);

  // case of drawing one pixel - optimized part
  if (left == right && top == bottom) {
    SPITransmitDataWord(dc, handle, color);
    xSemaphoreGive(mutex);
    return;
  }

  _u16 pixelsCountToFill = bottom - top + 1;

  for (_u16 x = left; x <= right; x++) {
    SPITransmitDataTimes(dc, handle, color, pixelsCountToFill);
  }
  xSemaphoreGive(mutex);
}

void Ili9341SelectRegion(_i8 dc, spi_device_handle_t handle, _u16 l, _u16 r,
                         _u16 t, _u16 b) {
  SPITransmitCommand(dc, handle, CASET);
  SPITransmitDataDWord(dc, handle, l, r);

  SPITransmitCommand(dc, handle, PASET);
  SPITransmitDataDWord(dc, handle, t, b);
}