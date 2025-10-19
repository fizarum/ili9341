# ili9341
Simple implementation of ili9341 driver for esp32

## usage
To communicate with ili9341 device you have to:
1. init & configure spi bus
2. create & configure *ILI9341_t* device
3. call __Ili9341Init()__ to initialize device
4. call __Ili9341PowerOn()__ to power on device

Then you can use rest of API.


### 1. init & configure spi bus
Usually, its about creation instance of *spi_bus_config_t* structure, configuring it. Then you have to call __spi_bus_initialize()__. After these steps you have to add spi device by configuring *spi_device_interface_config_t* structure and calling __spi_bus_add_device()__. Here is example of this step:
```
spi_bus_config_t busConfig = {
      .sclk_io_num = sclk,
      .mosi_io_num = mosi,
      .miso_io_num = -1,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
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

  spi_bus_add_device(host, &devConfig, handle);
```
You also can check example from esp-idf team [here](https://github.com/espressif/esp-idf/blob/v5.3.2/examples/peripherals/spi_master/hd_eeprom/main/spi_eeprom_main.c)


### 2. create & configure *ILI9341_t* device
*ILI9341_t* structure requires some definitions of display (like its size, rotation, pins etc) and pointers on functions for transmitting command, data, and light level set. Here is an example of such configuration:
```
ILI9341_t dev = {
    .width = DISPLAY_WIDTH,
    .height = DISPLAY_HEIGHT,
    .rotation = Angle0,
    .fontRotaion = Angle0,
    .colorMode = ModeBGR,
    .dc = DISPLAY_DC,
    .res = DISPLAY_RESET,
    .bl = DISPLAY_BL,
    .transmitCommand = &transmitCommand,
    .transmitData = &transmitData,
    .transmitDataTimes = &transmitDataTimes,
    .lighten = &lighten,
};
```
where *transmitCommand* has such definition:

`bool transmitCommand(const _u8 command)`

*transmitData:*

`bool transmitData(const _u8* data, const size_t length)`

*transmitDataTimes:*

`bool transmitDataTimes(const _u16 value, const _u16 times)`

*lighten:*

`bool lighten(const uint8_t percents)`

Now we have to set pins *dc* and *res* as output and set them to low state:
```
GPIO_Config(dev.dc, GPIO_MODE_OUTPUT);
GPIO_Config(dev.res, GPIO_MODE_OUTPUT);

GPIO_Set(dev.dc, 0);
GPIO_Set(dev.res, 0);
```
And final step is to wait for 1 millisecond and set res to hight state:

`GPIO_Set(dev.res, 1);`

### 3 and 4. steps
Just call __Ili9341Init(&dev)__ for configured *ILI9341_t* device and __Ili9341PowerOn(&dev, enable)__ to power it on. You may want to set brightness level by call __dev.lighten(brightness)__.