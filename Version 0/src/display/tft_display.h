#ifndef TFT_DISPLAY_H
#define TFT_DISPLAY_H

#include <TFT_eSPI.h>

// This declares a global reference to the 'tft' object of type 'TFT_eSPI'.
// The actual 'tft' object is defined and initialized in the main source file.
// Using 'extern' allows this object to be shared across multiple files without redefinition.
// It tells the compiler that 'tft' is defined elsewhere, and the linker will resolve its location at compile time.
extern TFT_eSPI tft;

#endif