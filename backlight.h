#ifndef BACKLIGHT_H
#define BACKLIGHT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

bool BacklightSetValue(int8_t gpioPin, uint8_t percents);

#ifdef __cplusplus
}
#endif

#endif  // BACKLIGHT_H