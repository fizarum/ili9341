#include "custom_hal_spi.h"

#include <driver/gpio.h>
#include <driver/spi_common.h>
#include <hal/gpio_types.h>
#include <string.h>

#define DC_C 0
#define DC_D 1
#define SPI_BYTE_BUFF_MAX_SIZE 1024

static _u8 _spiBuffer[SPI_BYTE_BUFF_MAX_SIZE];
static spi_transaction_t _transaction;

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
      .miso_io_num = GPIO_NUM_NC,
      .quadwp_io_num = GPIO_NUM_NC,
      .quadhd_io_num = GPIO_NUM_NC,
      // transfer 80 lines of pixels (assume pixel is RGB565)
      // at most in one SPI transaction
      .max_transfer_sz = 320 * 80 * sizeof(uint16_t),
  };

  esp_err_t result = spi_bus_initialize(host, &busConfig, SPI_DMA_CH_AUTO);
  if (result != ESP_OK) {
    return result;
  }

  spi_device_interface_config_t devConfig = {
      .clock_speed_hz = SPI_MASTER_FREQ_40M,
      .spics_io_num = cs,
      .queue_size = 7,
      .flags = SPI_DEVICE_NO_DUMMY,
  };

  return spi_bus_add_device(host, &devConfig, handle);
}

esp_err_t SPITransmit(spi_device_handle_t handle, const _u8 *data,
                      const size_t length) {
  if (length <= 0) {
    return ESP_OK;
  }

  _transaction.flags = 0;
  _transaction.cmd = 0;
  _transaction.addr = 0;
  _transaction.length = length * 8;
  _transaction.tx_buffer = data;

  return spi_device_transmit(handle, &_transaction);
}

esp_err_t SPITransmitCommand(_i8 dc, spi_device_handle_t handle,
                             const _u8 cmd) {
  _spiBuffer[0] = cmd;
  gpio_set_level(dc, DC_C);

  return SPITransmit(handle, _spiBuffer, 1);
}

esp_err_t SPITransmitCommandWord(_i8 dc, spi_device_handle_t handle,
                                 const _u16 cmd) {
  _spiBuffer[0] = (cmd >> 8) & 0xff;
  _spiBuffer[1] = cmd & 0xff;
  gpio_set_level(dc, DC_C);

  return SPITransmit(handle, _spiBuffer, 2);
}

esp_err_t SPITransmitData(_i8 dc, spi_device_handle_t handle, const _u8 data) {
  _spiBuffer[0] = data;
  gpio_set_level(dc, DC_D);

  return SPITransmit(handle, _spiBuffer, 1);
}

esp_err_t SPITransmitDataArray(_i8 dc, spi_device_handle_t handle,
                               const _u8 *dataArray, const _u16 arraySize) {
  memcpy(_spiBuffer, dataArray, arraySize);
  gpio_set_level(dc, DC_D);

  return SPITransmit(handle, _spiBuffer, arraySize);
}

esp_err_t SPITransmitDataWord(_i8 dc, spi_device_handle_t handle,
                              const _u16 data) {
  _spiBuffer[0] = (data >> 8) & 0xff;
  _spiBuffer[1] = data & 0xff;
  gpio_set_level(dc, DC_D);

  return SPITransmit(handle, _spiBuffer, 2);
}

esp_err_t SPITransmitDataWordArray(_i8 dc, spi_device_handle_t handle,
                                   const _u16 *dataArray,
                                   const _u16 arraySize) {
  _u16 bytesCount = arraySize * 2;

  if (bytesCount >= SPI_BYTE_BUFF_MAX_SIZE) {
    return ESP_ERR_INVALID_ARG;
  }

  _u16 index = 0;

  for (_u16 i = 0; i < arraySize; i++) {
    _spiBuffer[index++] = (dataArray[i] >> 8) & 0xff;
    _spiBuffer[index++] = dataArray[i] & 0xff;
  }

  gpio_set_level(dc, DC_D);
  return SPITransmit(handle, _spiBuffer, bytesCount);
}

esp_err_t SPITransmitDataDWord(_i8 dc, spi_device_handle_t handle,
                               const _u16 left, _u16 right) {
  _spiBuffer[0] = (left >> 8) & 0xff;
  _spiBuffer[1] = left & 0xff;
  _spiBuffer[2] = (right >> 8) & 0xff;
  _spiBuffer[3] = right & 0xff;
  gpio_set_level(dc, DC_D);

  return SPITransmit(handle, _spiBuffer, 4);
}

esp_err_t SPITransmitDataTimes(_i8 dc, spi_device_handle_t handle, _u16 value,
                               _u16 times) {
  _u16 bytesCount = times * 2;

  if (bytesCount >= SPI_BYTE_BUFF_MAX_SIZE) {
    return ESP_ERR_INVALID_ARG;
  }

  _u16 index = 0;

  for (_u16 i = 0; i < times; i++) {
    _spiBuffer[index++] = (value >> 8) & 0xff;
    _spiBuffer[index++] = value & 0xff;
  }
  gpio_set_level(dc, DC_D);

  return SPITransmit(handle, _spiBuffer, bytesCount);
}