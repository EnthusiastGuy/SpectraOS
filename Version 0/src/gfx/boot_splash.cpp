#include <Arduino.h>
#include <config.h>
#include "boot_splash.h"
#include "display/AXS15231B.h"
#include "zxSpectrumDesignation.h"
#include "display/tft_display.h"

/*
 * Sinclair Logo Boot Animation
 * 
 * This file contains the code responsible for rendering an animated Sinclair logo 
 * on the TFT display using the Arduino framework and the TFT_eSPI library. The animation 
 * consists of drawing individual letters of the word "Sinclair" with timed sequences, 
 * followed by colored flag stripes. It also manages drawing a designation name, and 
 * resetting or clearing the display at appropriate intervals. A lot of magic numbers, sorry.
 * 
 * Key Components:
 * - Initializes a TFT_eSprite for off-screen rendering.
 * - Draws individual letters (S, I, N, C, L, A, I, R) progressively based on a global 
 *   animation index (`animIndex`).
 * - Renders a multi-colored flag underneath the letters and displays a designation name.
 * - Manages sprite memory and handles animation flow control, including resetting and 
 *   clearing the logo when necessary.
 * 
 * Usage:
 * - Call `drawBootSplash(int index, TFT_eSPI& tft)` in a loop to continuously animate the logo.
 * - The animation progresses over time, driven by the `animIndex`, which increments each frame.
 * 
 * Dependencies:
 * - Arduino framework
 * - TFT_eSPI library
 * - AXS15231B.h and DesignationName.h for additional display and naming functionality.
 * 
 * Notes:
 * - Ensure proper initialization of the TFT display before invoking the animation.
 * - The animation is rotated 90 degrees to fit the display layout.
 * 
 * Support:
 * - RGB 565 color picker: https://rgbcolorpicker.com/565
 * - Image2LCD. Convert images to .h header files: https://www.buydisplay.com/image2lcd
 * 16bit true colour, MSB First, RGB565, don't include head data, be sure to set max image size, save as .h file.
 */

TFT_eSprite sinclairLogoSprite = TFT_eSprite(&tft);
bool spriteInitialized = false; // Flag to ensure sprite is initialized only once
int animIndex = 0;              // Animation frame counter

const int LOGO_X = 48;
const int LOGO_Y = 40;          // Must absolutely be a multiple of 4, see below why
const int LOGO_WIDTH = 560;
const int LOGO_HEIGHT = 96;     // This too, multiple of 4

const int VERTICAL_OFFSET = 14;
const float DEFAULT_SPEED_RATIO = 3.0;  // Used to control the drawing speed of some letters that will finish drawing too fast otherwise

void InitSpriteOnce() {
    if (spriteInitialized)
        return;

    // The drawing is rotated by 90 degrees, so the logical height of the image becomes the width in memory.
    // Due to memory alignment requirements, the height (which maps to the memory width) must be a multiple of 4 bytes
    // because the display controller processes data in chunks of 4 bytes (word-aligned).
    // Additionally, the y-coordinate (which corresponds to the horizontal position in memory) must also be aligned
    // to a multiple of 4. Failing to adhere to this alignment will cause artifacts, as the memory accesses becomes
    // misaligned, leading to incorrect rendering
    sinclairLogoSprite.createSprite(LOGO_WIDTH, LOGO_HEIGHT);       // Logo area
    sinclairLogoSprite.setSwapBytes(1);                             // Set byte swapping for correct color rendering
    resetSplash();

    spriteInitialized = true;
}

void drawLetterPart(int x, int y) {
    sinclairLogoSprite.fillRect(x, y, 11, 11, COLORS::WHITE);
}

void drawFlagPart(int x, int y, uint16_t color) {
    sinclairLogoSprite.fillRect(x, y, 27, 1, color);
}

// Draw the letter 'S' with animation, using a delay
void drawLetterS(int x, int delay = 0) {
    int correctedAnimIndex = animIndex - delay;

    if (correctedAnimIndex < 0)
        return;     // Skip if animation hasn't reached this point

    // Draw each part of the letter based on the animation index
    if (correctedAnimIndex < 57) {
        drawLetterPart(x + 57 - correctedAnimIndex, VERTICAL_OFFSET);
    } else if (correctedAnimIndex < 72) {
        drawLetterPart(x + 0, correctedAnimIndex - 57 + VERTICAL_OFFSET);
    } else if (correctedAnimIndex < 129) {
        drawLetterPart(x + correctedAnimIndex - 72, 16 + VERTICAL_OFFSET);
    } else if (correctedAnimIndex < 144) {
        drawLetterPart(x + 57, 16 + correctedAnimIndex - 129 + VERTICAL_OFFSET);
    } else if (correctedAnimIndex < 202) {
        drawLetterPart(x + 57 - (correctedAnimIndex - 144), 32 + VERTICAL_OFFSET);
    }
}

// Draw the letter 'I' with animation, adjusting speed using the default speed ratio
void drawLetterI(int x, int delay = 0) {
    int correctedAnimIndex = animIndex - delay;

    if (correctedAnimIndex < 0)
        return;

    if (correctedAnimIndex < 33 * DEFAULT_SPEED_RATIO) {
        drawLetterPart(x, VERTICAL_OFFSET + correctedAnimIndex/DEFAULT_SPEED_RATIO);
    } else if (correctedAnimIndex < 33 * DEFAULT_SPEED_RATIO + 11 + 10) {
        drawLetterPart(x, 0);
    }
}

// Draw the letter 'N' with animation and delay
void drawLetterN(int x, int delay = 0) {
    int correctedAnimIndex = animIndex - delay;

    if (correctedAnimIndex < 0)
        return;

    if (correctedAnimIndex < 32) {
        drawLetterPart(x, VERTICAL_OFFSET + 32 - correctedAnimIndex);
    } else if (correctedAnimIndex < 32 + 55) {
        drawLetterPart(x + correctedAnimIndex - 32, VERTICAL_OFFSET);
    } else if (correctedAnimIndex < 32 + 55 + 33) {
        drawLetterPart(x + 55, VERTICAL_OFFSET + correctedAnimIndex - (32 + 55));
    }
}

// Draw the letter 'C' with animation and delay
void drawLetterC(int x, int delay = 0) {
    int correctedAnimIndex = animIndex - delay;

    if (correctedAnimIndex < 0)
        return;

    if (correctedAnimIndex < 57) {               // Draw top part : 67 x 11 pixels
        drawLetterPart(x + 57 - correctedAnimIndex, VERTICAL_OFFSET);
    } else if (correctedAnimIndex < 90) {
        drawLetterPart(x + 0, correctedAnimIndex - 57 + VERTICAL_OFFSET);
    } else if (correctedAnimIndex < 145) {
        drawLetterPart(x + 57 + (correctedAnimIndex - 145), 32 + VERTICAL_OFFSET);
    }
}

// Draw the letter 'L' with animation and delay
void drawLetterL(int x, int delay = 0) {
    int correctedAnimIndex = animIndex - delay;

    if (correctedAnimIndex < 0)
        return;

    if (correctedAnimIndex < 47 * DEFAULT_SPEED_RATIO) {
        drawLetterPart(x, correctedAnimIndex/DEFAULT_SPEED_RATIO);
    }
}

// Draw the letter 'A' with animation and delay
void drawLetterA(int x, int delay = 0) {
    int correctedAnimIndex = animIndex - delay;

    if (correctedAnimIndex < 0)
        return;

    if (correctedAnimIndex < 57) {
        drawLetterPart(x + correctedAnimIndex, VERTICAL_OFFSET);
    } else if (correctedAnimIndex < 90) {
        drawLetterPart(x + 57, correctedAnimIndex - 57 + VERTICAL_OFFSET);
    } else if (correctedAnimIndex < 147) {
        drawLetterPart(x + 147 - correctedAnimIndex, 32 + VERTICAL_OFFSET);
    } else if (correctedAnimIndex < 162) {
        drawLetterPart(x, 16 + 162 - correctedAnimIndex + VERTICAL_OFFSET);
    } else if (correctedAnimIndex < 220) {
        drawLetterPart(x + (correctedAnimIndex - 162), 16 + VERTICAL_OFFSET);
    }
}

// Draw the letter 'R' with animation and delay
void drawLetterR(int x, int delay = 0) {
    int correctedAnimIndex = animIndex - delay;

    if (correctedAnimIndex < 0)
        return;

    if (correctedAnimIndex < 32) {
        drawLetterPart(x, VERTICAL_OFFSET + 32 - correctedAnimIndex);
    } else if (correctedAnimIndex < 32 + 55) {
        drawLetterPart(x + correctedAnimIndex - 32, VERTICAL_OFFSET);
    }
}

void drawFlagStripe(int x, uint16_t color, int delay = 0) {
    int correctedAnimIndex = animIndex - delay;

    if (correctedAnimIndex < 0)
        return;

    if (correctedAnimIndex < 96) {
        drawFlagPart(x + 96 - correctedAnimIndex / 2, correctedAnimIndex, color);
    }
}

void drawControlLine() {
    if (animIndex < 50) {
        drawLetterPart(animIndex * 10, 58);
    }
}

// Draw the "ZX Spectrum +3 Elite" name for the Sinclair logo with animation delay
void drawModelName(int delay = 0) {
    int correctedAnimIndex = animIndex - delay;

    if (correctedAnimIndex < 0)
        return;

    if (correctedAnimIndex < 23) {
        sinclairLogoSprite.pushImage(0, 70 + (23 - correctedAnimIndex), 281, 23, ZXSpectrumDesignation);
    }
}

void drawBootSplash() {
    InitSpriteOnce();
    int charactersDelay = 10;

    // Draw each letter of "SINCLAIR" with individual delays for animation
    drawLetterS(0);
    drawLetterI(73, charactersDelay);
    drawLetterN(89, charactersDelay * 2);
    drawLetterC(160, charactersDelay * 3);
    drawLetterL(232, charactersDelay * 4);
    drawLetterA(248, charactersDelay * 5);
    drawLetterI(321, charactersDelay * 6);
    drawLetterR(337, charactersDelay * 7);

    // Draw the flag stripes next to the letters
    drawFlagStripe(352, COLORS::RED, 96);
    drawFlagStripe(352 + 27, COLORS::YELLOW, 128);
    drawFlagStripe(352 + 27 * 2, COLORS::GREEN, 160);
    drawFlagStripe(352 + 27 * 3, COLORS::CYAN, 192);

    drawModelName(230);
    //drawControlLine();    // For tests

    animIndex += 1;

    // Push the sprite to the display
    lcd_PushColors_rotated_90(LOGO_X, LOGO_Y, LOGO_WIDTH, LOGO_HEIGHT, (uint16_t*)sinclairLogoSprite.getPointer());
}

void resetSplash() {
    sinclairLogoSprite.fillSprite(COLORS::BLACK);
    animIndex = 0;
}
