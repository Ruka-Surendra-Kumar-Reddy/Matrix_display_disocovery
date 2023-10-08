/*
 * DMD.h
 *
 *  Created on: Sep 3, 2023
 *      Author: Radiogeet_1
 */

#ifndef SRC_DMDLIB_DMD_H_
#define SRC_DMDLIB_DMD_H_


extern SPI_HandleTypeDef hspi1;
#define DMD_PIXELS_ACROSS         32      //pixels across x axis (base 2 size expected)
#define DMD_PIXELS_DOWN           16      //pixels down y axis
#define DMD_BITSPERPIXEL           1      //1 bit per pixel, use more bits to allow for pwm screen brightness control
#define DMD_RAM_SIZE_BYTES        ((DMD_PIXELS_ACROSS*DMD_BITSPERPIXEL/8)*DMD_PIXELS_DOWN)
#define LIGHT_DMD_ROW_01_05_09_13()       {  HAL_GPIO_WritePin(GPIOA, B_Pin, GPIO_PIN_RESET); HAL_GPIO_WritePin(GPIOA, A_Pin, GPIO_PIN_RESET);  }
#define LIGHT_DMD_ROW_02_06_10_14()       { HAL_GPIO_WritePin(GPIOA, B_Pin, GPIO_PIN_RESET);HAL_GPIO_WritePin(GPIOA, A_Pin, GPIO_PIN_SET); }
#define LIGHT_DMD_ROW_03_07_11_15()       { HAL_GPIO_WritePin(GPIOA, B_Pin, GPIO_PIN_SET); HAL_GPIO_WritePin(GPIOA, A_Pin, GPIO_PIN_RESET); }
#define LIGHT_DMD_ROW_04_08_12_16()       {HAL_GPIO_WritePin(GPIOA, B_Pin, GPIO_PIN_SET);HAL_GPIO_WritePin(GPIOA, A_Pin, GPIO_PIN_SET);}
#define LATCH_DMD_SHIFT_REG_TO_OUTPUT()   {     HAL_GPIO_WritePin(GPIOA, SCLK_Pin, GPIO_PIN_SET);  HAL_GPIO_WritePin(GPIOA, SCLK_Pin, GPIO_PIN_RESET); }
#define OE_DMD_ROWS_OFF()                 {    HAL_GPIO_WritePin(GPIOA, OE_Pin, GPIO_PIN_RESET);}
#define OE_DMD_ROWS_ON()                  {    HAL_GPIO_WritePin(GPIOA, OE_Pin, GPIO_PIN_SET); }

typedef uint8_t byte;
#define GRAPHICS_NORMAL    0
#define GRAPHICS_INVERSE   1
#define GRAPHICS_TOGGLE    2
#define GRAPHICS_OR        3
#define GRAPHICS_NOR       4
#define FONT_LENGTH             0
#define FONT_FIXED_WIDTH        2
#define FONT_HEIGHT             3
#define FONT_FIRST_CHAR          4
#define FONT_CHAR_COUNT         5
#define FONT_WIDTH_TABLE        6

#define PATTERN_ALT_0     0
#define PATTERN_ALT_1     1
#define PATTERN_STRIPE_0  2
#define PATTERN_STRIPE_1  3





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

extern  void MX_SPI1_Init(void);
void scanDisplayBySPI();
void drawString(int bX, int bY, const char *bChars, byte length, byte bGraphicsMode);
int drawChar(const int bX, const int bY, const unsigned char letter, byte bGraphicsMode);
int charWidth(const unsigned char letter);
void drawFilledBox(int x1, int y1, int x2, int y2,byte bGraphicsMode);
void selectFont(const uint8_t * font);
void drawLine(int x1, int y1, int x2, int y2, byte bGraphicsMode);
void writePixel(unsigned int bX, unsigned int bY, byte bGraphicsMode, byte bPixel);
void DMD(uint8_t panelsWide, uint8_t panelsHigh);
void clearScreen(uint8_t bNormal);
void drawBox(int x1, int y1, int x2, int y2, byte bGraphicsMode);
#endif /* SRC_DMDLIB_DMD_H_ */
