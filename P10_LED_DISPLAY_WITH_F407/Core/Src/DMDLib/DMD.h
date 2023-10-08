/*
 * DMD.h
 *
 *  Created on: Sep 3, 2023
 *      Author: Radiogeet_1
 */

#ifndef SRC_DMDLIB_DMD_H_
#define SRC_DMDLIB_DMD_H_

typedef uint8_t byte;

#define PIN_DMD_nOE       9    // D9 active low Output Enable, setting this low lights all the LEDs in the selected rows. Can pwm it at very high frequency for brightness control.
#define PIN_DMD_A         6    // D6
#define PIN_DMD_B         7    // D7
#define PIN_DMD_CLK       13   // D13_SCK  is SPI Clock if SPI is used
#define PIN_DMD_SCLK      8    // D8
#define PIN_DMD_R_DATA    11   // D11_MOSI is SPI Master Out if SPI is used
//Define this chip select pin that the Ethernet W5100 IC or other SPI device uses
//if it is in use during a DMD scan request then scanDisplayBySPI() will exit without conflict! (and skip that scan)
#define PIN_OTHER_SPI_nCS 10
// ######################################################################################################################
// ######################################################################################################################

//DMD I/O pin macros
#define LIGHT_DMD_ROW_01_05_09_13()       { digitalWrite( PIN_DMD_B,  LOW ); digitalWrite( PIN_DMD_A,  LOW ); }
#define LIGHT_DMD_ROW_02_06_10_14()       { digitalWrite( PIN_DMD_B,  LOW ); digitalWrite( PIN_DMD_A, HIGH ); }
#define LIGHT_DMD_ROW_03_07_11_15()       { digitalWrite( PIN_DMD_B, HIGH ); digitalWrite( PIN_DMD_A,  LOW ); }
#define LIGHT_DMD_ROW_04_08_12_16()       { digitalWrite( PIN_DMD_B, HIGH ); digitalWrite( PIN_DMD_A, HIGH ); }
#define LATCH_DMD_SHIFT_REG_TO_OUTPUT()   { digitalWrite( PIN_DMD_SCLK, HIGH ); digitalWrite( PIN_DMD_SCLK,  LOW ); }
#define OE_DMD_ROWS_OFF()                 { digitalWrite( PIN_DMD_nOE, LOW  ); }
#define OE_DMD_ROWS_ON()                  { digitalWrite( PIN_DMD_nOE, HIGH ); }

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
static byte bPixelLookupTable[8] =
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

void DMD(byte panelsWide, byte panelsHigh);

#endif /* SRC_DMDLIB_DMD_H_ */
