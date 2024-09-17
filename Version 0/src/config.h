#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Macro for RGB565 color format
#define RGB565(r, g, b)  ((r & 0x1F) << 11 | (g & 0x3F) << 5 | (b & 0x1F))

// Actually, this display is 180 by 640, but since we're using it horizontaly, we'll abstract
// and call the constants the other way around.
const int LCD_WIDTH         = 640;
const int LCD_HEIGHT        = 180;

const int TOUCH_TIMEOUT     = 30000;    // Timeout for resetting the touch state

// Colors                       Red: [0, 31], Green: [0, 63], Blue: [0, 31]
struct COLORS {
    static const uint16_t BLACK         = RGB565(0, 0, 0);
    static const uint16_t WHITE         = RGB565(31, 63, 31);
    static const uint16_t RED           = RGB565(31, 0, 0);
    static const uint16_t YELLOW        = RGB565(31, 63, 0);
    static const uint16_t GREEN         = RGB565(0, 55, 0);
    static const uint16_t CYAN          = RGB565(0, 63, 31);
};

#endif
