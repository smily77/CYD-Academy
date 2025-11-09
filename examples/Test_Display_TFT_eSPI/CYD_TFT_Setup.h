/*
  TFT_eSPI Setup für CYD 2.8" USB-C

  Basierend auf LovyanGFX Konfiguration.
  Diese Datei liegt im Sketch-Ordner (nicht in der Library!).
*/

#ifndef CYD_TFT_SETUP_H
#define CYD_TFT_SETUP_H

// ===== DISPLAY DRIVER =====
#define ILI9341_DRIVER
#define ILI9341_2_DRIVER  // Alternative Variante

// ===== DISPLAY AUFLÖSUNG =====
#define TFT_WIDTH  240
#define TFT_HEIGHT 320

// ===== SPI PINS =====
#define TFT_MISO 12
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC   2
#define TFT_RST  -1  // Nicht verwendet

// ===== BACKLIGHT =====
#define TFT_BL   21
#define TFT_BACKLIGHT_ON HIGH

// ===== SPI FREQUENZ =====
#define SPI_FREQUENCY       27000000  // 27 MHz (sicherer als 40MHz)
#define SPI_READ_FREQUENCY  16000000  // 16 MHz Read
#define SPI_TOUCH_FREQUENCY  2500000  // 2.5 MHz Touch

// ===== RGB ORDER =====
#define TFT_RGB_ORDER TFT_BGR  // Entspricht rgb_order = true in LovyanGFX

// ===== INVERT DISPLAY =====
// #define TFT_INVERSION_ON  // Auskommentiert = invert false
// #define TFT_INVERSION_OFF

// ===== FARBEN =====
#define TFT_BLACK       0x0000
#define TFT_NAVY        0x000F
#define TFT_DARKGREEN   0x03E0
#define TFT_DARKCYAN    0x03EF
#define TFT_MAROON      0x7800
#define TFT_PURPLE      0x780F
#define TFT_OLIVE       0x7BE0
#define TFT_LIGHTGREY   0xC618
#define TFT_DARKGREY    0x7BEF
#define TFT_BLUE        0x001F
#define TFT_GREEN       0x07E0
#define TFT_CYAN        0x07FF
#define TFT_RED         0xF800
#define TFT_MAGENTA     0xF81F
#define TFT_YELLOW      0xFFE0
#define TFT_WHITE       0xFFFF
#define TFT_ORANGE      0xFDA0
#define TFT_GREENYELLOW 0xB7E0
#define TFT_PINK        0xFE19
#define TFT_BROWN       0x9A60
#define TFT_GOLD        0xFEA0
#define TFT_SILVER      0xC618
#define TFT_SKYBLUE     0x867D
#define TFT_VIOLET      0x915C

// ===== FONT =====
#define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font
#define LOAD_FONT2  // Font 2. Small 16 pixel high font
#define LOAD_FONT4  // Font 4. Medium 26 pixel high font

#define SMOOTH_FONT

#endif // CYD_TFT_SETUP_H
