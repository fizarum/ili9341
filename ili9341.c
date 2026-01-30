#include "ili9341.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

static SemaphoreHandle_t mutex;
#define BUFFER_SIZE 1024
static _u8 buffer[BUFFER_SIZE];
static TickType_t _2 = pdTICKS_TO_MS(2);

void Ili9341SelectRegion(ILI9341_t* dev, _u16 l, _u16 r, _u16 t, _u16 b);
static void Uint16_ToByteArray(_u16 data);
static void Uint32_ToByteArray(_u16 first, _u16 second);
static void Uint16Array_ToByteArray(_u16* array, size_t arraySize);

void Ili9341Init(ILI9341_t* dev) {
  mutex = xSemaphoreCreateMutex();

  dev->transmitCommand(POWER_CTRL1);
  // SPITransmitData(dc, handle, 0x23);
  /**
  0x26 - 4.75V
  0x1F - 4.4V
  0x17 - 4.0V
  0x11 - 3.7V
  0x09 - 3.3V
  */
  _u8 ctrl1Data[1] = {0x26};
  dev->transmitData(ctrl1Data, sizeof(ctrl1Data));

  dev->transmitCommand(POWER_CTRL2);
  //  lowest value and default
  // AVDD=VCIx2, VGH=VCIx7, VGL=-VCIx3
  _u8 ctrl2Data[1] = {0x01};
  dev->transmitData(ctrl2Data, sizeof(ctrl2Data));

  dev->transmitCommand(VCOM_CTRL1);
  /**
  VMH: 0x1E = 3.45V
  0x35 = 5.225V
  0x3E = 5.850V
  VML: 0x28 = -1.500V
  */
  _u8 vcomCtrl1[2] = {0x35, 0x28};
  dev->transmitData(vcomCtrl1, sizeof(vcomCtrl1));

  dev->transmitCommand(VCOM_CTRL2);
  // VCOMH/VCOML voltage adjustment
  // VMH – 2 VML – 2
  _u8 vcomCtrl2[1] = {0xbe};
  dev->transmitData(vcomCtrl2, sizeof(vcomCtrl2));

  dev->transmitCommand(BCKL_CTRL7);
  // 0xff - 245Hz
  // 0x04 - 12.549 KHz
  _u8 br7[1] = {0x04};
  dev->transmitData(br7, sizeof(br7));

  // just give some time to apply some changes, when mcu has high
  // clock speed (240 MHz, for example) this part works incorrectly
  // without delay
  vTaskDelay(pdMS_TO_TICKS(10));

  Ili9341Rotate(dev, Angle270);

  dev->transmitCommand(PIXSET);
  // 65K color: 16-bit/pixel
  _u8 pixSet[1] = {0x55};
  dev->transmitData(pixSet, sizeof(pixSet));

  // Ili9341SetInversion(dev, false);

  /**
  Frame Rate:
  0x1B = 70 Hz
  0x19 = 76 Hz
  0x15 = 90 Hz
  0x13 = 100 Hz
  */
  dev->transmitCommand(FRMCTR1);
  _u8 frmCtrl1[2] = {0x00, 0x1B};
  dev->transmitData(frmCtrl1, sizeof(frmCtrl1));

  /**
   REV:1 - liqud crystal normally white
   GS:0 SS:1 SM:0
   SS:1, means S720 -> S1 (because of rotated display)
   ISC = 0x01 - scan cycle 3 frames (51 ms)
   PT = 0x02: Determine source/VCOM output in a non-display area in the partial
      display mode.
   PTG = 0x02: interval scan
   */
  _u8 disCtrl[4] = {0x0A, 0xA1, 0x27, 0x00};
  dev->transmitCommand(DISCTRL);
  dev->transmitData(disCtrl, sizeof(disCtrl));

  dev->transmitCommand(PGAMCTRL);
  _u8 pgc[15] = {
      0x1F, 0x1A, 0x18, 0x0A, 0x0F, 0x06, 0x45, 0x87,
      0x32, 0x0A, 0x07, 0x02, 0x07, 0x05, 0x00,
  };
  dev->transmitData(pgc, sizeof(pgc));

  dev->transmitCommand(NGAMCTRL);
  _u8 ngc[15] = {
      0x00, 0x25, 0x27, 0x05, 0x10, 0x09, 0x3A, 0x78,
      0x4D, 0x05, 0x18, 0x0D, 0x38, 0x3A, 0x1F,
  };
  dev->transmitData(ngc, sizeof(ngc));

  dev->transmitCommand(SLPOUT);
  /**
   * During the Resetting period, the display will be blanked
   * (The display is entering blanking sequence, which maximum time
   * is 120 ms, when Reset Starts in Sleep Out –mode.
   *
   * Check datasheet, page 218, "15.4. Reset Timing"
   */
  vTaskDelay(pdMS_TO_TICKS(121));
  dev->transmitCommand(DISPON);
}

bool Ili9341PowerOn(ILI9341_t* dev, bool on) {
  xSemaphoreTake(mutex, portMAX_DELAY);

  bool result = dev->transmitCommand(on == true ? DISPON : DISPOFF);

  xSemaphoreGive(mutex);
  return result;
}

bool ILI9341Sleep(ILI9341_t* dev) {
  bool result = dev->transmitCommand(SLPIN);
  vTaskDelay(pdMS_TO_TICKS(130));
  return result;
}

bool ILI9341Wakeup(ILI9341_t* dev) {
  bool result = dev->transmitCommand(SLPOUT);
  vTaskDelay(pdMS_TO_TICKS(130));
  return result;
}

bool Ili9341Rotate(ILI9341_t* dev, const Rotation_t rotation) {
  xSemaphoreTake(mutex, portMAX_DELAY);

  if (dev->rotation == rotation) {
    xSemaphoreGive(mutex);
    return true;
  }

  _u8 mx, my, mv = 0x00;

  bool isOldModePortrait =
      (dev->rotation == Angle0) || (dev->rotation == Angle180);
  bool isNewModePortait = (rotation == Angle0) || (rotation == Angle180);

  dev->transmitCommand(MADCTL);

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
  }
  dev->rotation = rotation;

  _u8 rotData[1] = {mx | my | mv | (dev->colorMode)};
  bool result = dev->transmitData(rotData, sizeof(rotData));

  xSemaphoreGive(mutex);

  return result;
}

bool Ili9341SetInversion(ILI9341_t* dev, const bool inversionOn) {
  xSemaphoreTake(mutex, portMAX_DELAY);
  bool result = dev->transmitCommand(inversionOn == true ? DINVON : DINVOFF);

  xSemaphoreGive(mutex);
  return result;
}

bool Ili9341SetColorMode(ILI9341_t* dev, const ColorMode_t mode) {
  xSemaphoreTake(mutex, portMAX_DELAY);

  dev->colorMode = mode;
  dev->transmitCommand(MADCTL);
  _u8 modeData[1] = {mode};
  bool result = dev->transmitData(modeData, sizeof(modeData));

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
void Ili9341SetScrollArea(ILI9341_t* dev, _u16 tfa, _u16 vsa, _u16 bfa) {
  xSemaphoreTake(mutex, _2);

  dev->transmitCommand(VSCRDEF);

  Uint16_ToByteArray(tfa);
  dev->transmitData(buffer, 2);

  Uint16_ToByteArray(vsa);
  dev->transmitData(buffer, 2);

  Uint16_ToByteArray(bfa);
  dev->transmitData(buffer, 2);

  xSemaphoreGive(mutex);
}

void Ili9341ResetScrollArea(ILI9341_t* dev, _u16 vsa) {
  xSemaphoreTake(mutex, _2);

  dev->transmitCommand(VSCRDEF);

  Uint16_ToByteArray(0);
  dev->transmitData(buffer, 2);

  Uint16_ToByteArray(vsa);
  dev->transmitData(buffer, 2);

  Uint16_ToByteArray(0);
  dev->transmitData(buffer, 2);

  xSemaphoreGive(mutex);
}

/**
 * @brief Vertical Scrolling Start Address
 *
 * @param vsp Vertical Scrolling Start Address, the line in the
 * Frame Memory that will be written as the first line after
 * the last line of the Top Fixed Area on the display
 */
void Ili9341Scroll(ILI9341_t* dev, _u16 vsp) {
  xSemaphoreTake(mutex, _2);
  dev->transmitCommand(VSCRSADD);

  Uint16_ToByteArray(vsp);
  dev->transmitData(buffer, 2);

  xSemaphoreGive(mutex);
}

void Ili9341DrawPixel(ILI9341_t* dev, _u16 left, _u16 top, _u16 color) {
  Ili9341DrawPixelTimes(dev, left, top, left, top, color);
}

// TODO: Add 8 bit version
void Ili9341DrawPixels(ILI9341_t* dev, _u16 left, _u16 top, _u16 right,
                       _u16 bottom, _u16* colors, size_t colorsSize) {
  if (colorsSize >= BUFFER_SIZE / 2) {
    return;
  }

  if (left >= dev->width) return;
  if (top >= dev->height) return;

  if (right >= dev->width) {
    right = dev->width - 1;
  }

  if (bottom >= dev->height) {
    bottom = dev->height - 1;
  }

  xSemaphoreTake(mutex, _2);
  Ili9341SelectRegion(dev, left, right, top, bottom);
  dev->transmitCommand(RAMWR);

  Uint16Array_ToByteArray(colors, colorsSize);
  dev->transmitData(buffer, colorsSize * 2);
  xSemaphoreGive(mutex);
}

/**
 * @brief Fill display region by provided color
 */
void Ili9341DrawPixelTimes(ILI9341_t* dev, _u16 left, _u16 top, _u16 right,
                           _u16 bottom, _u16 color) {
  if (left >= dev->width) return;
  if (top >= dev->height) return;

  if (right >= dev->width) {
    right = dev->width - 1;
  }

  if (bottom >= dev->height) {
    bottom = dev->height - 1;
  }

  xSemaphoreTake(mutex, _2);

  Ili9341SelectRegion(dev, left, right, top, bottom);
  dev->transmitCommand(RAMWR);

  // case of drawing one pixel - optimized part
  if (left == right && top == bottom) {
    Uint16_ToByteArray(color);
    dev->transmitData(buffer, 2);
    xSemaphoreGive(mutex);
    return;
  }

  _u16 pixelsCountToFill = bottom - top + 1;

  for (_u16 x = left; x <= right; x++) {
    dev->transmitDataTimes(color, pixelsCountToFill);
  }
  xSemaphoreGive(mutex);
}

void Ili9341SelectRegion(ILI9341_t* dev, _u16 l, _u16 r, _u16 t, _u16 b) {
  dev->transmitCommand(CASET);
  Uint32_ToByteArray(l, r);
  dev->transmitData(buffer, 4);

  dev->transmitCommand(PASET);
  Uint32_ToByteArray(t, b);
  dev->transmitData(buffer, 4);
}

static void Uint16_ToByteArray(_u16 data) {
  buffer[0] = (data >> 8) & 0xff;
  buffer[1] = data & 0xff;
}

static void Uint16Array_ToByteArray(_u16* array, size_t arraySize) {
  _u16 data;
  size_t outputArrayIndex = 0;
  for (size_t index = 0; index < arraySize; index++) {
    data = array[index];

    buffer[outputArrayIndex] = ((data >> 8) & 0xff);
    outputArrayIndex++;
    buffer[outputArrayIndex] = (data & 0xff);
    outputArrayIndex++;
  }
}

static void Uint32_ToByteArray(_u16 first, _u16 second) {
  buffer[0] = (first >> 8) & 0xff;
  buffer[1] = first & 0xff;
  buffer[2] = (second >> 8) & 0xff;
  buffer[3] = second & 0xff;
}