/**
 * Spectra OS - Operating System for the ZX Spectrum +3 Elite (A hardware modified version of the ZX Spectrum +3)
 * --------------------------------------------------------------------------------------------------------------
 * 
 * Project: Spectra OS
 * Version: 0.0
 * 
 * Description:
 * Spectra OS is (will be) a custom operating system designed to bring modern functionality
 * to the classic ZX Spectrum +3, building upon the legacy of both Sinclair and Amstrad.
 * Running on an LILYGO ESP32 T-Display S3 Long microcontroller (180px x 640px)
 * https://www.lilygo.cc/products/t-display-s3-long, Spectra OS provides seamless
 * integration with advanced hardware features while preserving the nostalgic essence of
 * retro computing.
 * 
 * Core Features:
 * - SD Card Support: Load and save disk images (.dsk), audio files, and other media directly to/from an SD card.
 * - Audio Playback and Recording: Control tape emulation, audio playback, and recording functions with high fidelity.
 * - Keyboard Macro System: Automate complex key sequences, simplifying tasks like resetting the machine, switching modes, and loading files.
 * - Advanced File Management: Browse and manage files stored on the SD card through a modern interface displayed on the onboard screen.
 * - System Control: Manage hardware settings including video output, audio levels, and even advanced system resets.
 * - Customizable Boot Process: Load specific configurations, including switching between 48K BASIC and other modes.
 * - Enhanced User Experience: Introduces a modern, user-friendly interface that enhances the functionality of the ZX Spectrum +3 while remaining true to its roots.
 * - Other...
 * 
 * Vision:
 * Spectra OS aims to breathe new life into the ZX Spectrum +3, offering a fusion of retro computing nostalgia
 * and modern-day convenience. From seamless file handling to automated macro commands, Spectra OS is designed
 * for retro enthusiasts who want to push the ZX Spectrum +3 beyond its original capabilities, while maintaining
 * the charm and simplicity that made the original system iconic.
 * 
 * Developed By: Enthusiast Guy
 * Inspired By: The ZX Spectrum, Sinclair, Amstrad, and Retro Computing Community
 * 
 */

#include <Arduino.h>
#include <Wire.h>               // Wire library for I2C communication
#include <TFT_eSPI.h>           // TFT_eSPI library for handling the display

#include "display/AXS15231B.h"  // Custom display driver header
#include "pins_config.h"        // Pin configurations
#include "gfx/boot_splash.h"
#include "config.h"

TFT_eSPI tft = TFT_eSPI();      // Initialize the display object

void setup()
{
    pinMode(TOUCH_INT, INPUT_PULLUP);   // Set the touch interrupt pin as input with pull-up resistor

    // Comment this out if using variable brightness
    pinMode(TFT_BL, OUTPUT);            // Set backlight pin as output
    digitalWrite(TFT_BL, HIGH);         // Turn on backlight

    // Initialize touch screen 
    pinMode(TOUCH_RES, OUTPUT);                 // Set touch reset pin as output
    digitalWrite(TOUCH_RES, HIGH); delay(2);    // Reset sequence for the touch controller
    digitalWrite(TOUCH_RES, LOW); delay(10);
    digitalWrite(TOUCH_RES, HIGH); delay(2);
    Wire.begin(TOUCH_IICSDA, TOUCH_IICSCL);     // Start I2C communication for touch controller

    axs15231_init();                    // Initialize display

    lcd_fill(0, 0, LCD_HEIGHT, LCD_WIDTH, COLORS::BLACK);     // Clear the screen to black, initially
}

void loop() 
{
    drawBootSplash();

    if (digitalRead(TOUCH_INT) == LOW) 
    {
        resetSplash();
    }    
}
