#include "custom_hal_spi.h"

#include <driver/gpio.h>
#include <driver/spi_common.h>
#include <hal/gpio_types.h>
#include <string.h>

#include "freertos/semphr.h"

#define DC_C 0
#define DC_D 1
#define SPI_BYTE_BUFF_MAX_SIZE 1024

static SemaphoreHandle_t mutex;

static _u8 _spi_buffer[SPI_BYTE_BUFF_MAX_SIZE];

esp_err_t PinSetAsOutput(const _i8 pin, const _u8 state) {
  if (pin < 0) {
    return ESP_OK;
  }

  esp_err_t result = ESP_OK;

  result = gpio_reset_pin(pin);
  if (result != ESP_OK) {
    return result;
  }

  result = gpio_set_direction(pin, GPIO_MODE_OUTPUT);
  if (result != ESP_OK) {
    return result;
  }

  result = gpio_set_level(pin, state);
  if (result != ESP_OK) {
    return result;
  }

  return ESP_OK;
}

esp_err_t SPIInit(spi_device_handle_t *handle, spi_host_device_t host, _u8 mosi,
                  _u8 sclk, _u8 cs) {
  spi_bus_config_t busConfig = {
      .sclk_io_num = sclk,
      .mosi_io_num = mosi,
      .miso_io_num = -1,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
  };

  esp_err_t result = spi_bus_initialize(host, &busConfig, SPI_DMA_CH_AUTO);
  if (result != ESP_OK) {
    return result;
  }

  spi_device_interface_config_t devConfig = {
      .clock_speed_hz = SPI_MASTER_FREQ_10M,
      .spics_io_num = cs,
      .queue_size = 7,
      .flags = SPI_DEVICE_NO_DUMMY,
  };

  mutex = xSemaphoreCreateMutex();

  return spi_bus_add_device(host, &devConfig, handle);
}

esp_err_t SPITransmit(spi_device_handle_t handle, const _u8 *data,
                      const size_t length) {
  if (length <= 0) {
    return ESP_OK;
  }

  xSemaphoreTake(mutex, portMAX_DELAY);

  spi_transaction_t transaction = {
      .flags = 0,
      .cmd = 0,
      .addr = 0,
      .length = length * 8,
      .tx_buffer = data,
  };

  esp_err_t result = spi_device_transmit(handle, &transaction);
  xSemaphoreGive(mutex);

  return result;
}

esp_err_t SPITransmitCommand(_i8 dc, spi_device_handle_t handle,
                             const _u8 cmd) {
  static _u8 byte = 0;
  byte = cmd;
  gpio_set_level(dc, DC_C);

  return SPITransmit(handle, &byte, 1);
}

esp_err_t SPITransmitCommandWord(_i8 dc, spi_device_handle_t handle,
                                 const _u16 cmd) {
  static _u8 bytes[2];

  bytes[0] = (cmd >> 8) & 0xff;
  bytes[1] = cmd & 0xff;
  gpio_set_level(dc, DC_C);

  return SPITransmit(handle, bytes, 2);
}

esp_err_t SPITransmitData(_i8 dc, spi_device_handle_t handle, const _u8 data) {
  static _u8 byte = 0;
  byte = data;
  gpio_set_level(dc, DC_D);

  return SPITransmit(handle, &byte, 1);
}

esp_err_t SPITransmitDataWord(_i8 dc, spi_device_handle_t handle,
                              const _u16 data) {
  static _u8 bytes[2];

  bytes[0] = (data >> 8) & 0xff;
  bytes[1] = data & 0xff;
  gpio_set_level(dc, DC_D);

  return SPITransmit(handle, bytes, 2);
}

esp_err_t SPITransmitDataDWord(_i8 dc, spi_device_handle_t handle,
                               const _u16 left, _u16 right) {
  static _u8 bytes[4];
  bytes[0] = (left >> 8) & 0xff;
  bytes[1] = left & 0xff;
  bytes[2] = (right >> 8) & 0xff;
  bytes[3] = right & 0xff;
  gpio_set_level(dc, DC_D);

  return SPITransmit(handle, bytes, 4);
}

esp_err_t SPITransmitDataTimes(_i8 dc, spi_device_handle_t handle, _u16 value,
                               _u16 times) {
  _u16 bytesCount = times * 2;

  if (bytesCount >= SPI_BYTE_BUFF_MAX_SIZE) {
    return ESP_ERR_INVALID_ARG;
  }

  memset(_spi_buffer, 0, SPI_BYTE_BUFF_MAX_SIZE);

  // static _u8 bytes[SPI_BYTE_BUFF_MAX_SIZE];
  _u16 index = 0;

  for (_u16 i = 0; i < times; i++) {
    _spi_buffer[index++] = (value >> 8) & 0xff;
    _spi_buffer[index++] = value & 0xff;
  }
  gpio_set_level(dc, DC_D);

  return SPITransmit(handle, _spi_buffer, bytesCount);
}

esp_err_t SPITransmitDataArray(_i8 dc, spi_device_handle_t handle, _u16 *array,
                               _u16 length) {
  _u16 bytesCount = length * 2;

  if (bytesCount >= SPI_BYTE_BUFF_MAX_SIZE) {
    return ESP_ERR_INVALID_ARG;
  }

  memset(_spi_buffer, 0, SPI_BYTE_BUFF_MAX_SIZE);

  _u16 index = 0;

  for (_u16 i = 0; i < length; i++) {
    _spi_buffer[index++] = (array[i] >> 8) & 0xff;
    _spi_buffer[index++] = array[i] & 0xff;
  }

  gpio_set_level(dc, DC_D);
  return SPITransmit(handle, _spi_buffer, bytesCount);
}