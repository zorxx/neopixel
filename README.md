# ESP32 Neopixel Driver
High-performance low-level driver for WS2812B LEDs, allowing individual control of hundreds of LEDs via a single ESP32 GPIO.

Add this component to an esp-idf project with the following command:
```bash
idf.py add-dependency "zorxx/neopixel"
```

Source for this project lives at [https://github.com/zorxx/neopixel](https://github.com/zorxx/neopixel)

# Overview
This library makes use of the ESP32 [I2S](https://en.wikipedia.org/wiki/I%C2%B2S) peripheral bus to
control "any" number of WS2812b LEDs, consuming minimal ESP32 resources.

Some of the highlights include:
* Simple abstract API
* Use of I2S [DMA](https://en.wikipedia.org/wiki/Direct_memory_access) and double-buffering to minimize ESP32 CPU usage.
* A high-speed lookup table implementation for setting pixel colors.
* WS2812b data bus runs at maximum-supported data rate of 2.6 Mbps.

# Performance
In order to perform a pixel update, the WS2812b protocol requires 9 bytes per pixel plus 50 bytes at the end of the transmission. For a string of 256 pixels, this equates to 2354 bytes per transfer and at 2.6 Mbps, each update requires approximately 7.3ms. With a configuration of 256 LEDs, this library supports refresh rates over 130 "frames" per second. With 512 LEDs, a refresh rate of nearly 70 "frames" per second is achieved. 

With this library's double-buffer implementation, pixel buffer updates (via a `neopixel_SetPixel` function call) occur asynchronously and in a thread-safe manner with respect to the I2S hardware transfers. If a pixel update is in progress when the pixel buffer is updated, another update will begin immediately following the completion of the pixel update. If no pixel update is in progress when the pixel buffer is updated, the I2S hardware transfer will begin immediately.

Unlike other WS2812b pixel buffer update routines which make use of many expensive bitwise operations to build the pixel buffer for each update, this library utilizes a lookup table implementation to update colors for only the LEDs that have changed since the last update. The lookup table contains pre-computed values, indexed by a color's desired intensity (0-255). This requires only a copy of 9 bytes to change a pixel's RGB value.

# Example Applications
See the `examples` directory of this repository for example Neopixel applications.

# License
All files delivered with this library are copyright 2023 Zorxx Software and released under the MIT license. See the `LICENSE` file for details.
