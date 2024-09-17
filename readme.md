## Spectra OS
Spectra OS is a modern operating system designed for the ZX Spectrum +3 Elite, running on the ESP32-S3 microcontroller for T-Display S3 Long device. The device is meant to take the place of the floppy drive and manage multiple devices in relation with the ZX Spectrum itself, such as a Gotek, a video converter, audio and many others. This project gradually evolves, adding new features while maintaining access to historical versions. This allows users to explore or use smaller, more containable versions of the operating system as it grows.

## Project Structure and Versioning
The Spectra OS project uses by design an unusual approach to versioning, where each new feature is built on top of a complete, functional base. Instead of overwriting the previous versions, each version is maintained in its own folder, allowing users to access the exact state of the project at various stages of development.

## How It Works:
Version 0: The starting point, which is the minimal functional feature, in this case a simple splash screen.
Version 0.1: This version builds upon Version 0, introducing new features or improvements, but still includes the functionality of Version 0.
Version 0.2, 0.3, etc.: Each subsequent version represents an incremental step in the development of Spectra OS. New features will be added, but each version may retain the core functionality of its predecessors, modify or even remove some functionality.

Major Versions: When significant milestones are reached, the version number will increment accordingly (e.g., Version 1.0, 2.0).

## Why This Approach?
Historical Tracking: Each version of Spectra OS is preserved for easy access, allowing developers and users to observe the evolution of the project over time.
Choose Your Version: Viewers can choose a smaller, more contained version of the project (such as Version 0.1) if they want a simpler, more focused experience without the complexity of later versions.
Gradual Learning: Newcomers can start with early versions to gradually learn the system as it becomes more complex, rather than jumping into a large, multi-featured version.

## Repository Structure
```
/SpectraOS
  ├── Version 0         # Initial version (basic setup, splash screen)
  ├── Version 0.1       # Adds functionality to Version 0
  ├── Version 0.2       # Builds upon Version 0.1 with additional features
  ├── Version 0.3       # Further developments (e.g., adding file management or audio control)
  └── ...
```

## Dependencies
- Bodmer/TFT_eSPI

## How to Use
Clone the repository:

```
git clone https://github.com/EnthusiastGuy/SpectraOS.git
```

Use Visual Studio Code with the PlatformIO extension: Import your project (Version 0 or some other version) through the PlatformIO homepage (make sure you select the lilygo-t-display-s3 board when importing), connect the device through USB, build and push to device.

Build and upload that version to your ESP32-S3 device as per the instructions in each version’s README.md file.

## Contributions
This is an open-source project. My programming skills are a bit rusty on C++ since my main area of work takes C# and this might be showing. If you'd like to contribute, feel free to fork the repo and submit a pull request with improvements.

## Thank you!
A big thank you to NikTheFix who wrote a beautiful calculator for the T-Display. I found it very inspirational and was able to quickly get something simple going
reading his code. Check out his repo here: https://github.com/nikthefix/Lilygo_Support_T_Display_S3_Long_TFT_eSPI_Volos-nikthefix


## MIT License

Copyright (c) [2024] [Enthusiast Guy]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

This project includes the logos of the Sinclair ZX Spectrum and Amstrad. The copyright for these logos belongs to their respective owners:

The Sinclair ZX Spectrum logo is owned by Sky Group (Sky UK Limited), the current rights holder.
The Amstrad logo is owned by Amstrad (Amstrad Consumer Electronics plc).
These logos are included for reference and historical purposes only. This project does not claim
any ownership over these logos, and they are used in compliance with their respective copyright laws.
