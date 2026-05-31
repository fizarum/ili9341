# ili9341
Simple implementation of ili9341 driver for esp32

## dependencies
This library uses __mini_hal_esp32__(https://github.com/fizarum/mini_hal_esp32) so make sure you have it installed locally. if you're using PlatformIO you can add this dependency pretty easy:
```
lib_deps = 
    ;hal
    https://github.com/fizarum/mini_hal_esp32.git
```

## usage
To communicate with ili9341 device you have to:
1. init & configure spi bus
2. create & configure *ILI9341_t* device
3. call __Ili9341Init()__ to initialize device
4. call __Ili9341PowerOn()__ to power on device

Then you can use rest of API.


### 1. init & configure spi bus
To initialize spi bus just call __spi_init()__ with `mosi`, `sclk` gpio parameters. 


### 2. create & configure *spi_display_t* device
Now we need to add a display as spi device. __spi_add_device()__ adds new display. You can add multiple displays on same bus, but make sure you have provided correct `cs` gpio for each one. Finally call __display_init()__ which initialize ili9341 display.

That's it!