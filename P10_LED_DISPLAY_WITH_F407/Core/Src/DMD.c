#include "DMD.h"
#include "stdlib.h"
#include "string.h"
#include "stdint.h"
#include "stdbool.h"
#include "stm32f4xx_hal.h"
// Define your SPI peripheral and handle
extern SPI_HandleTypeDef hspi1;
// Define your font and other constants
extern const uint8_t Font[];  // Define your font array her
// Define your display properties
//#define DisplaysWide   4  // Number of DMD panels wide
//#define DisplaysHigh   1  // Number of DMD panels high
#define DMD_PIXELS_ACROSS 32
#define DMD_PIXELS_DOWN 16

int data;
// Define your pattern constants
#define PATTERN_ALT_0 0
#define PATTERN_ALT_1 1
#define PATTERN_STRIPE_0 2
#define PATTERN_STRIPE_1 3

// Define macros for DMD control
int rowsizevalue;
int offsevalue;
int displayvalue;

// Define your DMD structure

// Function prototypes
// Define your SPI transfer function (use HAL_SPI_Transmit)
void SPI_transfer(uint8_t data);

// Implement the SPI transfer function
void SPI_transfer(uint8_t data) {
    HAL_SPI_Transmit(&hspi1, &data, 1, HAL_MAX_DELAY);
}

// Implementation of DMD functions
void DMD_init(DMD *dmd, uint8_t panelsWide, uint8_t panelsHigh) {
    uint16_t ui;
    dmd->DisplaysWide = panelsWide;
    dmd->DisplaysHigh = panelsHigh;
    dmd->DisplaysTotal = panelsWide * panelsHigh;
    dmd->row1 = dmd->DisplaysTotal << 4;
    dmd->row2 	= dmd->DisplaysTotal << 5;
    dmd->row3 = ((dmd->DisplaysTotal << 2) * 3) << 2;
    dmd->bDMDScreenRAM = (uint8_t *)malloc(DMD_RAM_SIZE_BYTES * dmd->DisplaysTotal);

    // Initialize the SPI port
    HAL_SPI_Init(&hspi1);  // Make sure to configure SPI settings as needed
    HAL_GPIO_WritePin(GPIOA, PIN_DMD_A | PIN_DMD_B | PIN_DMD_CLK | PIN_DMD_SCLK | PIN_DMD_nOE, GPIO_PIN_RESET);
HAL_GPIO_WritePin(GPIOA, PIN_DMD_R_DATA , 1);
    DMD_clearScreen(dmd, true);

    // Init the scan line/ram pointer to the required start point
    dmd->bDMDByte = 0;
}

void DMD_writePixel(DMD *dmd, uint16_t bX, uint16_t bY, uint8_t bGraphicsMode, uint8_t bPixel) {
    uint16_t uiDMDRAMPointer;

    if (bX >= (DMD_PIXELS_ACROSS * dmd->DisplaysWide) || bY >= (DMD_PIXELS_DOWN * dmd->DisplaysHigh)) {
        return;
    }
    uint8_t panel = (bX / DMD_PIXELS_ACROSS) + (dmd->DisplaysWide * (bY / DMD_PIXELS_DOWN));
    bX = (bX % DMD_PIXELS_ACROSS) + (panel << 5);
    bY = bY % DMD_PIXELS_DOWN;
    // Set pointer to DMD RAM byte to be modified
    uiDMDRAMPointer = bX / 8 + bY * (dmd->DisplaysTotal << 2);

    uint8_t lookup = bPixelLookupTable[bX & 0x07];

    switch (bGraphicsMode) {
        case GRAPHICS_NORMAL:
            if (bPixel == true)
                dmd->bDMDScreenRAM[uiDMDRAMPointer] &= ~lookup;    // zero bit is pixel on
            else
                dmd->bDMDScreenRAM[uiDMDRAMPointer] |= lookup;    // one bit is pixel off
            break;
        case GRAPHICS_INVERSE:
            if (bPixel == false)
                dmd->bDMDScreenRAM[uiDMDRAMPointer] &= ~lookup;    // zero bit is pixel on
            else
                dmd->bDMDScreenRAM[uiDMDRAMPointer] |= lookup;    // one bit is pixel off
            break;
        case GRAPHICS_TOGGLE:
            if (bPixel == true) {
                if ((dmd->bDMDScreenRAM[uiDMDRAMPointer] & lookup) == 0)
                    dmd->bDMDScreenRAM[uiDMDRAMPointer] |= lookup;    // one bit is pixel off
                else
                    dmd->bDMDScreenRAM[uiDMDRAMPointer] &= ~lookup;    // one bit is pixel off
            }
            break;
        case GRAPHICS_OR:
            // Only set pixels on
            if (bPixel == true)
                dmd->bDMDScreenRAM[uiDMDRAMPointer] &= ~lookup;    // zero bit is pixel on
            break;
        case GRAPHICS_NOR:
            // Only clear on pixels
            if ((bPixel == true) &&
                ((dmd->bDMDScreenRAM[uiDMDRAMPointer] & lookup) == 0))
                dmd->bDMDScreenRAM[uiDMDRAMPointer] |= lookup;    // one bit is pixel off
            break;
    }
}



void DMD_drawString(DMD *dmd, int bX, int bY, const char *bChars, uint8_t length, uint8_t bGraphicsMode) {
    if (bX >= (DMD_PIXELS_ACROSS * dmd->DisplaysWide) || bY >= (DMD_PIXELS_DOWN * dmd->DisplaysHigh))
        return;
    uint8_t height = dmd->Font[FONT_HEIGHT];
    if (bY + height < 0)
        return;

    int strWidth = 0;
    DMD_drawLine(dmd, bX - 1, bY, bX - 1, bY + height, GRAPHICS_INVERSE);

    for (int i = 0; i < length; i++) {
        int charWide = DMD_drawChar(dmd, bX + strWidth, bY, bChars[i], bGraphicsMode);
        if (charWide > 0) {
            strWidth += charWide;
            DMD_drawLine(dmd, bX + strWidth, bY, bX + strWidth, bY + height, GRAPHICS_INVERSE);
            strWidth++;
        } else if (charWide < 0) {
            return;
        }
        if ((bX + strWidth) >= DMD_PIXELS_ACROSS * dmd->DisplaysWide || bY >= DMD_PIXELS_DOWN * dmd->DisplaysHigh)
            return;
    }
}


void DMD_drawMarquee(DMD *dmd, const char *bChars, uint8_t length, int left, int top) {
    dmd->marqueeWidth = 0;
    for (int i = 0; i < length; i++) {
        dmd->marqueeText[i] = bChars[i];
        dmd->marqueeWidth += DMD_charWidth(dmd->Font, bChars[i]) + 1;
    }
    dmd->marqueeHeight = pgm_read_byte(dmd->Font + FONT_HEIGHT);
    dmd->marqueeText[length] = '\0';
    dmd->marqueeOffsetY = top;
    dmd->marqueeOffsetX = left;
    dmd->marqueeLength = length;
    DMD_drawString(dmd, dmd->marqueeOffsetX, dmd->marqueeOffsetY, dmd->marqueeText, dmd->marqueeLength, GRAPHICS_NORMAL);
}

bool DMD_stepMarquee(DMD *dmd, int amountX, int amountY) {
    bool ret = false;
    dmd->marqueeOffsetX += amountX;
    dmd->marqueeOffsetY += amountY;
    if (dmd->marqueeOffsetX < -dmd->marqueeWidth) {
        dmd->marqueeOffsetX = DMD_PIXELS_ACROSS * dmd->DisplaysWide;
        DMD_clearScreen(dmd, true);
        ret = true;
    } else if (dmd->marqueeOffsetX > DMD_PIXELS_ACROSS * dmd->DisplaysWide) {
        dmd->marqueeOffsetX = -dmd->marqueeWidth;
        DMD_clearScreen(dmd, true);
        ret = true;
    }

    if (dmd->marqueeOffsetY < -dmd->marqueeHeight) {
        dmd->marqueeOffsetY = DMD_PIXELS_DOWN * dmd->DisplaysHigh;
        DMD_clearScreen(dmd, true);
        ret = true;
    } else if (dmd->marqueeOffsetY > DMD_PIXELS_DOWN * dmd->DisplaysHigh) {
        dmd->marqueeOffsetY = -dmd->marqueeHeight;
        DMD_clearScreen(dmd, true);
        ret = true;
    }

    // Special case horizontal scrolling to improve speed
    if (amountY == 0 && amountX == -1) {
        // Shift entire screen one bit
        for (int i = 0; i < DMD_RAM_SIZE_BYTES * dmd->DisplaysTotal; i++) {
            if ((i % (dmd->DisplaysWide * 4)) == (dmd->DisplaysWide * 4) - 1) {
                dmd->bDMDScreenRAM[i] = (dmd->bDMDScreenRAM[i] << 1) + 1;
            } else {
                dmd->bDMDScreenRAM[i] = (dmd->bDMDScreenRAM[i] << 1) + ((dmd->bDMDScreenRAM[i + 1] & 0x80) >> 7);
            }
        }

        // Redraw last char on screen
        int strWidth = dmd->marqueeOffsetX;
        for (uint8_t i = 0; i < dmd->marqueeLength; i++) {
            int wide = DMD_charWidth(dmd->Font, dmd->marqueeText[i]);
            if (strWidth + wide >= DMD_PIXELS_ACROSS * dmd->DisplaysWide) {
                DMD_drawChar(dmd, strWidth, dmd->marqueeOffsetY, dmd->marqueeText[i], GRAPHICS_NORMAL);
                return ret;
            }
            strWidth += wide + 1;
        }
    } else if (amountY == 0 && amountX == 1) {
        // Shift entire screen one bit
        for (int i = (DMD_RAM_SIZE_BYTES * dmd->DisplaysTotal) - 1; i >= 0; i--) {
            if ((i % (dmd->DisplaysWide * 4)) == 0) {
                dmd->bDMDScreenRAM[i] = (dmd->bDMDScreenRAM[i] >> 1) + 128;
            } else {
                dmd->bDMDScreenRAM[i] = (dmd->bDMDScreenRAM[i] >> 1) + ((dmd->bDMDScreenRAM[i - 1] & 1) << 7);
            }
        }

        // Redraw last char on screen
        int strWidth = dmd->marqueeOffsetX;
        for (uint8_t i = 0; i < dmd->marqueeLength; i++) {
            int wide = DMD_charWidth(dmd->Font, dmd->marqueeText[i]);
            if (strWidth + wide >= 0) {
                DMD_drawChar(dmd, strWidth, dmd->marqueeOffsetY, dmd->marqueeText[i], GRAPHICS_NORMAL);
                return ret;
            }
            strWidth += wide + 1;
        }
    } else {
        DMD_drawString(dmd, dmd->marqueeOffsetX, dmd->marqueeOffsetY, dmd->marqueeText, dmd->marqueeLength, GRAPHICS_NORMAL);
    }

    return ret;
}

/*--------------------------------------------------------------------------------------
 Clear the screen in DMD RAM
--------------------------------------------------------------------------------------*/
void DMD_clearScreen(DMD *dmd, uint8_t bNormal) {
    if (bNormal) // clear all pixels
        memset(dmd->bDMDScreenRAM, 0xFF, DMD_RAM_SIZE_BYTES * dmd->DisplaysTotal);
    else // set all pixels
        memset(dmd->bDMDScreenRAM, 0x00, DMD_RAM_SIZE_BYTES * dmd->DisplaysTotal);
}

/*--------------------------------------------------------------------------------------
 Draw or clear a line from x1,y1 to x2,y2
--------------------------------------------------------------------------------------*/
void DMD_drawLine(DMD *dmd, int x1, int y1, int x2, int y2, uint8_t bGraphicsMode) {
    int dy = y2 - y1;
    int dx = x2 - x1;
    int stepx, stepy;

    if (dy < 0) {
        dy = -dy;
        stepy = -1;
    } else {
        stepy = 1;
    }
    if (dx < 0) {
        dx = -dx;
        stepx = -1;
    } else {
        stepx = 1;
    }
    dy <<= 1;            // dy is now 2*dy
    dx <<= 1;            // dx is now 2*dx

    DMD_writePixel(dmd, x1, y1, bGraphicsMode, true);
    if (dx > dy) {
        int fraction = dy - (dx >> 1);    // same as 2*dy - dx
        while (x1 != x2) {
            if (fraction >= 0) {
                y1 += stepy;
                fraction -= dx;    // same as fraction -= 2*dx
            }
            x1 += stepx;
            fraction += dy;    // same as fraction -= 2*dy
            DMD_writePixel(dmd, x1, y1, bGraphicsMode, true);
        }
    } else {
        int fraction = dx - (dy >> 1);
        while (y1 != y2) {
            if (fraction >= 0) {
                x1 += stepx;
                fraction -= dy;
            }
            y1 += stepy;
            fraction += dx;
            DMD_writePixel(dmd, x1, y1, bGraphicsMode, true);
        }
    }
}
/*--------------------------------------------------------------------------------------
 Draw or clear a circle of radius r at x,y centre
--------------------------------------------------------------------------------------*/

void DMD_drawCircleSub(DMD *dmd, int cx, int cy, int x, int y,uint8_t bGraphicsMode) {
    if (x == 0) {
        DMD_writePixel(dmd, cx, cy + y, bGraphicsMode, true);
        DMD_writePixel(dmd, cx, cy - y, bGraphicsMode, true);
        DMD_writePixel(dmd, cx + y, cy, bGraphicsMode, true);
        DMD_writePixel(dmd, cx - y, cy, bGraphicsMode, true);
    } else if (x == y) {
        DMD_writePixel(dmd, cx + x, cy + y, bGraphicsMode, true);
        DMD_writePixel(dmd, cx - x, cy + y, bGraphicsMode, true);
        DMD_writePixel(dmd, cx + x, cy - y, bGraphicsMode, true);
        DMD_writePixel(dmd, cx - x, cy - y, bGraphicsMode, true);
    } else if (x < y) {
        DMD_writePixel(dmd, cx + x, cy + y, bGraphicsMode, true);
        DMD_writePixel(dmd, cx - x, cy + y, bGraphicsMode, true);
        DMD_writePixel(dmd, cx + x, cy - y, bGraphicsMode, true);
        DMD_writePixel(dmd, cx - x, cy - y, bGraphicsMode, true);
        DMD_writePixel(dmd, cx + y, cy + x, bGraphicsMode, true);
        DMD_writePixel(dmd, cx - y, cy + x, bGraphicsMode, true);
        DMD_writePixel(dmd, cx + y, cy - x, bGraphicsMode, true);
        DMD_writePixel(dmd, cx - y, cy - x, bGraphicsMode, true);
    }
}
void DMD_drawCircle(DMD *dmd, int xCenter, int yCenter, int radius, uint8_t bGraphicsMode) {
    int x = 0;
    int y = radius;
    int p = (5 - radius * 4) / 4;

    DMD_drawCircleSub(dmd, xCenter, yCenter, x, y, bGraphicsMode);
    while (x < y) {
        x++;
        if (p < 0) {
            p += 2 * x + 1;
        } else {
            y--;
            p += 2 * (x - y) + 1;
        }
        DMD_drawCircleSub(dmd, xCenter, yCenter, x, y, bGraphicsMode);
    }
}

/*--------------------------------------------------------------------------------------
 Draw or clear a box(rectangle) with a single pixel border
--------------------------------------------------------------------------------------*/
void DMD_drawBox(DMD *dmd, int x1, int y1, int x2, int y2, uint8_t bGraphicsMode) {
    DMD_drawLine(dmd, x1, y1, x2, y1, bGraphicsMode);
    DMD_drawLine(dmd, x2, y1, x2, y2, bGraphicsMode);
    DMD_drawLine(dmd, x2, y2, x1, y2, bGraphicsMode);
    DMD_drawLine(dmd, x1, y2, x1, y1, bGraphicsMode);
}

/*--------------------------------------------------------------------------------------
 Draw or clear a filled box(rectangle) with a single pixel border
--------------------------------------------------------------------------------------*/
void DMD_drawFilledBox(DMD *dmd, int x1, int y1, int x2, int y2, uint8_t bGraphicsMode) {
    for (int b = x1; b <= x2; b++) {
        DMD_drawLine(dmd, b, y1, b, y2, bGraphicsMode);
    }
}

/*--------------------------------------------------------------------------------------
 Draw the selected test pattern
--------------------------------------------------------------------------------------*/
void DMD_drawTestPattern(DMD *dmd, uint8_t bPattern) {
    unsigned int ui;

    int numPixels = dmd->DisplaysTotal * DMD_PIXELS_ACROSS * DMD_PIXELS_DOWN;
    int pixelsWide = DMD_PIXELS_ACROSS * dmd->DisplaysWide;
    for (ui = 0; ui < numPixels; ui++) {
        switch (bPattern) {
            case PATTERN_ALT_0:    // every alternate pixel, first pixel on
                if ((ui & pixelsWide) == 0)
                    //even row
                    DMD_writePixel(dmd, (ui & (pixelsWide - 1)), ((ui & ~(pixelsWide - 1)) / pixelsWide), GRAPHICS_NORMAL, ui & 1);
                else
                    //odd row
                    DMD_writePixel(dmd, (ui & (pixelsWide - 1)), ((ui & ~(pixelsWide - 1)) / pixelsWide), GRAPHICS_NORMAL, !(ui & 1));
                break;
            case PATTERN_ALT_1:    // every alternate pixel, first pixel off
                if ((ui & pixelsWide) == 0)
                    //even row
                    DMD_writePixel(dmd, (ui & (pixelsWide - 1)), ((ui & ~(pixelsWide - 1)) / pixelsWide), GRAPHICS_NORMAL, !(ui & 1));
                else
                    //odd row
                    DMD_writePixel(dmd, (ui & (pixelsWide - 1)), ((ui & ~(pixelsWide - 1)) / pixelsWide), GRAPHICS_NORMAL, ui & 1);
                break;
            case PATTERN_STRIPE_0:    // vertical stripes, first stripe on
                DMD_writePixel(dmd, (ui & (pixelsWide - 1)), ((ui & ~(pixelsWide - 1)) / pixelsWide), GRAPHICS_NORMAL, ui & 1);
                break;
            case PATTERN_STRIPE_1:    // vertical stripes, first stripe off
                DMD_writePixel(dmd, (ui & (pixelsWide - 1)), ((ui & ~(pixelsWide - 1)) / pixelsWide), GRAPHICS_NORMAL, !(ui & 1));
                break;
        }
    }
}

/*--------------------------------------------------------------------------------------
 Scan the dot matrix LED panel display, from the RAM mirror out to the display hardware.
 Call 4 times to scan the whole display which is made up of 4 interleaved rows within the 16 total rows.
 Insert the calls to this function into the main loop for the highest call rate, or from a timer interrupt
--------------------------------------------------------------------------------------*/
void DMD_scanDisplayBySPI(DMD *dmd) {
    //if PIN_OTHER_SPI_nCS is in use during a DMD scan request then scanDisplayBySPI() will exit without conflict! (and skip that scan)
	if (HAL_GPIO_ReadPin(DMD_PORT, PIN_OTHER_SPI_nCS) == GPIO_PIN_SET) {
	        // SPI transfer pixels to the display hardware shift registers
		int rowsize= dmd->DisplaysTotal << 2;
	  rowsizevalue = rowsize;
	      int offset = rowsize * dmd->bDMDByte;
	      offsevalue = dmd->bDMDByte;
	      displayvalue = dmd->DisplaysTotal;

	        for (int i = 0; i<rowsize; i++) {
	                   HAL_SPI_Transmit(&hspi1,&dmd->bDMDScreenRAM[offset + i + dmd->row3], sizeof(dmd->bDMDScreenRAM[offset + i + dmd->row3]), HAL_MAX_DELAY);
	                   HAL_SPI_Transmit(&hspi1,&dmd->bDMDScreenRAM[offset + i + dmd->row2], sizeof(dmd->bDMDScreenRAM[offset + i + dmd->row2]), HAL_MAX_DELAY);
	                   HAL_SPI_Transmit(&hspi1,&dmd->bDMDScreenRAM[offset + i + dmd->row1], sizeof(dmd->bDMDScreenRAM[offset + i + dmd->row1]), HAL_MAX_DELAY);
	                   HAL_SPI_Transmit(&hspi1,&dmd->bDMDScreenRAM[offset + i], sizeof(dmd->bDMDScreenRAM[offset + i]), HAL_MAX_DELAY);

	               }

        OE_DMD_ROWS_OFF();
        LATCH_DMD_SHIFT_REG_TO_OUTPUT();
        switch (dmd->bDMDByte) {
            case 0:        // row 1, 5, 9, 13 were clocked out
                LIGHT_DMD_ROW_01_05_09_13();
                dmd->bDMDByte = 1;
                break;
            case 1:        // row 2, 6, 10, 14 were clocked out
                LIGHT_DMD_ROW_02_06_10_14();
                dmd->bDMDByte = 2;
                break;
            case 2:        // row 3, 7, 11, 15 were clocked out
                LIGHT_DMD_ROW_03_07_11_15();
                dmd->bDMDByte = 3;
                break;
            case 3:        // row 4, 8, 12, 16 were clocked out
                LIGHT_DMD_ROW_04_08_12_16();
                dmd->bDMDByte = 0;
                break;
        }
        OE_DMD_ROWS_ON();
    }
}

void DMD_selectFont(DMD *dmd, const uint8_t *font) {
    dmd->Font = font;
}

int DMD_drawChar(DMD *dmd, int bX, int bY, unsigned char letter, uint8_t bGraphicsMode) {
    if (bX > (DMD_PIXELS_ACROSS * dmd->DisplaysWide) || bY > (DMD_PIXELS_DOWN * dmd->DisplaysHigh)) return -1;
    unsigned char c = letter;
    uint8_t height = dmd->Font[FONT_HEIGHT];
    if (c == ' ') {
        int charWide = DMD_charWidth(dmd, ' ');
        DMD_drawFilledBox(dmd, bX, bY, bX + charWide, bY + height, GRAPHICS_INVERSE);
        return charWide;
    }
    uint8_t width = 0;
    uint8_t bytes = (height + 7) / 8;

    uint8_t firstChar = dmd->Font[FONT_FIRST_CHAR];
    uint8_t charCount = dmd->Font[FONT_CHAR_COUNT];

    uint16_t index = 0;

    if (c < firstChar || c >= (firstChar + charCount)) return 0;
    c -= firstChar;

    if (dmd->Font[FONT_LENGTH] == 0 && dmd->Font[FONT_LENGTH + 1] == 0) {
        // zero length is flag indicating fixed width font (array does not contain width data entries)
        width = dmd->Font[FONT_FIXED_WIDTH];
        index = c * bytes * width + FONT_WIDTH_TABLE;
    } else {
        // variable width font, read width data, to get the index
        for (uint8_t i = 0; i < c; i++) {
            index += dmd->Font[FONT_WIDTH_TABLE + i];
        }
        index = index * bytes + charCount + FONT_WIDTH_TABLE;
        width = dmd->Font[FONT_WIDTH_TABLE + c];
    }
    if (bX < -width || bY < -height) return width;

    // last but not least, draw the character
    for (uint8_t j = 0; j < width; j++) { // Width
        for (uint8_t i = bytes - 1; i < 254; i--) { // Vertical Bytes
            uint8_t data = dmd->Font[index + j + (i * width)];
            int offset = (i * 8);
            if ((i == bytes - 1) && bytes > 1) {
                offset = height - 8;
            }
            for (uint8_t k = 0; k < 8; k++) { // Vertical bits
                if ((offset + k >= i * 8) && (offset + k < i * 8 + 8)) {
                    if (data & (0x01 << k)) {
                        DMD_writePixel(dmd, bX + j, bY + offset + k, bGraphicsMode, true);
                    } else if (bGraphicsMode == GRAPHICS_NORMAL) {
                        DMD_writePixel(dmd, bX + j, bY + offset + k, GRAPHICS_NORMAL, false);
                    }
                }
            }
        }
    }
    return width;
}

int DMD_stringWidth(DMD *dmd, const char *s) {
    int strWidth = 0;
    char c;
    while ((c = *s++)) {
        int charWidth = DMD_charWidth(dmd, c);
        strWidth += charWidth;
    }
    return strWidth;
}

// Change the function prototype to match the definition
int DMD_charWidth(const uint8_t *font, unsigned char letter)
{
    unsigned char c = letter;
    // Space is often not included in font so use width of 'n'
    if (c == ' ')
        c = 'n';
    DMD *dmd;
    uint8_t width = 0;

    uint8_t firstChar = dmd->Font[FONT_FIRST_CHAR];
    uint8_t charCount = dmd->Font[FONT_CHAR_COUNT];
    uint16_t index = 0;

    if (c < firstChar || c >= (firstChar + charCount)) {
        return 0;
    }
    c -= firstChar;
    if (dmd->Font[FONT_LENGTH] == 0 && dmd->Font[FONT_LENGTH + 1] == 0) {
        // zero length is flag indicating fixed width font (array does not contain width data entries)
        width = dmd->Font[FONT_FIXED_WIDTH];
    } else {
        // variable width font, read width data
        width = dmd->Font[FONT_WIDTH_TABLE + c];
    }
    return width;
}

