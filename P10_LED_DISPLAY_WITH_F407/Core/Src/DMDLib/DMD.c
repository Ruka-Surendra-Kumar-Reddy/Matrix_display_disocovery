/*
 * DMD.c
 *
 *  Created on: Sep 3, 2023
 *      Author: Radiogeet_1
 */
#include "stdlib.h"
#include "main.h"
#include "stdint.h"
#include "stdbool.h"
#include "DMD.h"


uint32_t *bDMDScreenRAM;
//Marquee values
char marqueeText[256];
byte marqueeLength;
int marqueeWidth;
int marqueeHeight;
int marqueeOffsetX;
int marqueeOffsetY;
//Pointer to current font
const uint8_t* Font;
//Display information
byte DisplaysWide;
byte DisplaysHigh;
byte DisplaysTotal;
int row1, row2, row3;
//scanning pointer into bDMDScreenRAM, setup init @ 48 for the first valid scan
volatile byte bDMDByte;
void DMD(byte panelsWide, byte panelsHigh)
{
    uint16_t ui;
    DisplaysWide=panelsWide;
    DisplaysHigh=panelsHigh;
    DisplaysTotal=DisplaysWide*DisplaysHigh;
    row1 = DisplaysTotal<<4;
    row2 = DisplaysTotal<<5;
    row3 = ((DisplaysTotal<<2)*3)<<2;
    bDMDScreenRAM = (uint32_t *) malloc(DisplaysTotal*DMD_RAM_SIZE_BYTES);



    HAL_GPIO_WritePin(GPIOA, PIN_DMD_A | PIN_DMD_B | PIN_DMD_CLK | PIN_DMD_SCLK | PIN_DMD_nOE, GPIO_PIN_RESET);
   HAL_GPIO_WritePin(GPIOA, PIN_DMD_R_DATA , GPIO_PIN_SET);

//    clearScreen(true);
    // init the scan line/ram pointer to the required start point
    bDMDByte = 0;
}

