/*--------------------------------------------------------------------------------------

 DMD.h   - Function and support library for the Freetronics DMD, a 512 LED matrix display
           panel arranged in a 32 x 16 layout.

 Copyright (C) 2011 Marc Alexander (info <at> freetronics <dot> com)

 Note that the DMD library uses the SPI port for the fastest, low overhead writing to the
 display. Keep an eye on conflicts if there are any other devices running from the same
 SPI port, and that the chip select on those devices is correctly set to be inactive
 when the DMD is being written to.

LED Panel Layout in RAM
                            32 pixels (4 bytes)
        top left  ----------------------------------------
                  |                                      |
         Screen 1 |        512 pixels (64 bytes)         | 16 pixels
                  |                                      |
                  ---------------------------------------- bottom right

 ---

 This program is free software: you can redistribute it and/or modify it under the terms
 of the version 3 GNU General Public License as published by the Free Software Foundation.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program.
 If not, see <http://www.gnu.org/licenses/>.

--------------------------------------------------------------------------------------*/
#ifndef DMD_H_
#define DMD_H_
#include "stm32f4xx_hal.h"
#include <stdint.h>
#include<stdbool.h>
// ######################################################################################################################
// ######################################################################################################################

//Arduino pins used for the display connection
#define PIN_DMD_nOE       GPIO_PIN_1    // D9 active low Output Enable, setting this low lights all the LEDs in the selected rows. Can pwm it at very high frequency for brightness control.
#define PIN_DMD_A         GPIO_PIN_2      // D6
#define PIN_DMD_B         GPIO_PIN_3      // D7
#define PIN_DMD_CLK       GPIO_PIN_5     // D13_SCK  is SPI Clock if SPI is used
#define PIN_DMD_SCLK      GPIO_PIN_4      // D8
#define PIN_DMD_R_DATA    GPIO_PIN_7     // D11_MOSI is SPI Master Out if SPI is used
//Define this chip select pin that the Ethernet W5100 IC or other SPI device uses
//if it is in use during a DMD scan request then scanDisplayBySPI() will exit without conflict! (and skip that scan)
#define PIN_OTHER_SPI_nCS  GPIO_PIN_6
// ######################################################################################################################
// ######################################################################################################################
#define DMD_PORT GPIOA
//DMD I/O pin macros
#define LIGHT_DMD_ROW_01_05_09_13()       { \
    HAL_GPIO_WritePin(DMD_PORT, PIN_DMD_A, GPIO_PIN_RESET); \
    HAL_GPIO_WritePin(DMD_PORT, PIN_DMD_B, GPIO_PIN_RESET); \
}

#define LIGHT_DMD_ROW_02_06_10_14()       { \
    HAL_GPIO_WritePin(DMD_PORT, PIN_DMD_A, GPIO_PIN_SET); \
    HAL_GPIO_WritePin(DMD_PORT, PIN_DMD_B, GPIO_PIN_RESET); \
}

#define LIGHT_DMD_ROW_03_07_11_15()       { \
    HAL_GPIO_WritePin(DMD_PORT, PIN_DMD_A, GPIO_PIN_RESET); \
    HAL_GPIO_WritePin(DMD_PORT, PIN_DMD_B, GPIO_PIN_SET); \
}

#define LIGHT_DMD_ROW_04_08_12_16()       { \
    HAL_GPIO_WritePin(DMD_PORT, PIN_DMD_A, GPIO_PIN_SET); \
    HAL_GPIO_WritePin(DMD_PORT, PIN_DMD_B, GPIO_PIN_SET); \
}

#define LATCH_DMD_SHIFT_REG_TO_OUTPUT()   { \
    HAL_GPIO_WritePin(DMD_PORT, PIN_DMD_SCLK, GPIO_PIN_SET); \
    HAL_GPIO_WritePin(DMD_PORT, PIN_DMD_SCLK, GPIO_PIN_RESET); \
}

#define OE_DMD_ROWS_OFF()                 { \
    HAL_GPIO_WritePin(DMD_PORT, PIN_DMD_nOE, GPIO_PIN_RESET); \
}

#define OE_DMD_ROWS_ON()                  { \
    HAL_GPIO_WritePin(DMD_PORT, PIN_DMD_nOE, GPIO_PIN_SET); \
}

//Pixel/graphics writing modes (bGraphicsMode)
#define GRAPHICS_NORMAL    0
#define GRAPHICS_INVERSE   1
#define GRAPHICS_TOGGLE    2
#define GRAPHICS_OR        3
#define GRAPHICS_NOR       4

//drawTestPattern Patterns
#define PATTERN_ALT_0     0
#define PATTERN_ALT_1     1
#define PATTERN_STRIPE_0  2
#define PATTERN_STRIPE_1  3

//display screen (and subscreen) sizing
#define DMD_PIXELS_ACROSS         32      //pixels across x axis (base 2 size expected)
#define DMD_PIXELS_DOWN           16      //pixels down y axis
#define DMD_BITSPERPIXEL           1      //1 bit per pixel, use more bits to allow for pwm screen brightness control
#define DMD_RAM_SIZE_BYTES        ((DMD_PIXELS_ACROSS*DMD_BITSPERPIXEL/8)*DMD_PIXELS_DOWN)
                                  // (32x * 1 / 8) = 4 bytes, * 16y = 64 bytes per screen here.
//lookup table for DMD::writePixel to make the pixel indexing routine faster
static uint8_t bPixelLookupTable[8] =
{
   0x80,   //0, bit 7
   0x40,   //1, bit 6
   0x20,   //2. bit 5
   0x10,   //3, bit 4
   0x08,   //4, bit 3
   0x04,   //5, bit 2
   0x02,   //6, bit 1
   0x01    //7, bit 0
};

// Font Indices
#define FONT_LENGTH             0
#define FONT_FIXED_WIDTH        2
#define FONT_HEIGHT             3
#define FONT_FIRST_CHAR         4
#define FONT_CHAR_COUNT         5
#define FONT_WIDTH_TABLE        6

typedef uint8_t (*FontCallback)(const uint8_t*);

//The main structure of DMD library functions
// Define the DMD structure without its functions
typedef struct {
    // Add the fields of the DMD structure here
    // ...

    // Mirror of DMD pixels in RAM, ready to be clocked out by the main loop or high-speed timer calls
    uint8_t *bDMDScreenRAM;

    // Marquee values
    char marqueeText[256];
    uint8_t  marqueeLength;
    int marqueeWidth;
    int marqueeHeight;
    int marqueeOffsetX;
    int marqueeOffsetY;

    // Pointer to the current font
    const uint8_t* Font;

    // Display information
    uint8_t DisplaysWide;
    uint8_t DisplaysHigh;
    uint8_t DisplaysTotal;
    int row1, row2, row3;

    // Scanning pointer into bDMDScreenRAM, setup init @ 48 for the first valid scan
    volatile uint8_t bDMDByte;
} DMD;

// Instantiate the DMD
DMD* createDMD(uint8_t panelsWide, uint8_t panelsHigh);
void DMD_init(DMD *dmd, uint8_t panelsWide, uint8_t panelsHigh);
void DMD_writePixel(DMD *dmd, uint16_t bX, uint16_t bY, uint8_t bGraphicsMode, uint8_t bPixel);
void DMD_drawString(DMD *dmd, int bX, int bY, const char *bChars, uint8_t length, uint8_t bGraphicsMode);
void DMD_drawMarquee(DMD *dmd, const char *bChars, uint8_t length, int left, int top);
bool DMD_stepMarquee(DMD *dmd, int amountX, int amountY);
void DMD_clearScreen(DMD *dmd, uint8_t bNormal);
void DMD_drawLine(DMD *dmd, int x1, int y1, int x2, int y2, uint8_t bGraphicsMode);
void DMD_drawCircle(DMD *dmd, int xCenter, int yCenter, int radius, uint8_t bGraphicsMode);
void DMD_drawBox(DMD *dmd, int x1, int y1, int x2, int y2, uint8_t bGraphicsMode);
void DMD_drawFilledBox(DMD *dmd, int x1, int y1, int x2, int y2, uint8_t bGraphicsMode);
void DMD_drawTestPattern(DMD *dmd, uint8_t bPattern);
void DMD_scanDisplayBySPI(DMD *dmd);
void DMD_selectFont(DMD *dmd, const uint8_t *font);
int DMD_drawChar(DMD *dmd, int bX, int bY, unsigned char letter, uint8_t bGraphicsMode);
int DMD_charWidth(const uint8_t *font, unsigned char letter);

// Set or clear a pixel at the x and y location (0,0 is the top left corner)

#endif /* DMD_H_ */

