#ifndef CUSTOM_HAL_SPI_H
#define CUSTOM_HAL_SPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <driver/spi_master.h>
#include <esp_err.h>
#include <hal/spi_types.h>

typedef uint8_t _u8;
typedef int8_t _i8;
typedef uint16_t _u16;

typedef struct SPIDescriptor_t {
  const _u8 mosi;
  const _u8 sclk;
  const _u8 cs;
  spi_host_device_t host;
  spi_device_handle_t handle;
} SPIDescriptor_t;

esp_err_t PinSetAsOutput(const _i8 pin, const _u8 state);

esp_err_t SPIInit(spi_device_handle_t *handle, spi_host_device_t host, _u8 mosi,
                  _u8 sclk, _u8 cs);

esp_err_t SPITransmit(spi_device_handle_t handle, const _u8 *data,
                      const size_t length);

esp_err_t SPITransmitCommand(_i8 dc, spi_device_handle_t handle, const _u8 cmd);
esp_err_t SPITransmitCommandWord(_i8 dc, spi_device_handle_t handle,
                                 const _u16 cmd);

esp_err_t SPITransmitData(_i8 dc, spi_device_handle_t handle, const _u8 data);
esp_err_t SPITransmitDataArray(_i8 dc, spi_device_handle_t handle,
                               const _u8 *dataArray, const _u16 arraySize);
esp_err_t SPITransmitDataWord(_i8 dc, spi_device_handle_t handle,
                              const _u16 data);
esp_err_t SPITransmitDataWordArray(_i8 dc, spi_device_handle_t handle,
                                   const _u16 *dataArray, const _u16 arraySize);
esp_err_t SPITransmitDataDWord(_i8 dc, spi_device_handle_t handle,
                               const _u16 first, _u16 second);

esp_err_t SPITransmitDataTimes(_i8 dc, spi_device_handle_t handle, _u16 value,
                               _u16 times);

#ifdef __cplusplus
}
#endif

#endif  // CUSTOM_HAL_SPI_H
