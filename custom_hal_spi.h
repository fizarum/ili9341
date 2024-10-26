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

esp_err_t SPIInit(spi_device_handle_t *handle, spi_host_device_t host, _u8 mosi,
                  _u8 sclk, _u8 cs);

#ifdef __cplusplus
}
#endif

#endif  // CUSTOM_HAL_SPI_H
