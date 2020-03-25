/**
 * This Library was originally written by Olivier Van den Eede (4ilo) in 2016.
 * Some refactoring was done and SPI support was added by Aleksander Alekseev (afiskon) in 2018.
 * Updating to function pointer and rotating support were added by Duy Lion Tran in 2019.
 *
 * https://github.com/afiskon/stm32-ssd1309
 * https://github.com/DuyTrandeLion/nrf52-ssd1309
 */

#ifndef __SSD1309_H__
#define __SSD1309_H__

#include <stddef.h>

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "ssd1309_fonts.h"

#include "nordic_common.h"
#include "nrf.h"
#include "boards.h"
#include "nrf_gpio.h"
#include "app_util_platform.h"
#include "app_error.h"

#if defined(SSD1309_USE_I2C)
#define SSD1309_I2C_ADDR        0x3C

#define MAX_TX_DATA             64
#define MAX_ADDRESS_SIZE        2

#define MAX_TX_SIZE             (MAX_TX_DATA + MAX_ADDRESS_SIZE)
typedef void (*ssd1309_i2c_handle)(uint8_t, uint8_t *, size);
#elif defined(SSD1309_USE_SPI)
typedef void (*ssd1309_spi_handle)(uint8_t, uint8_t *, size_t);
#else
#error "You should define SSD1309_USE_SPI or SSD1309_USE_I2C macro!"
#endif

/* SSD1309 OLED height in pixels  */
#ifndef SSD1309_HEIGHT
#define SSD1309_HEIGHT          64
#endif

/* SSD1309 width in pixels	  */
#ifndef SSD1309_WIDTH
#define SSD1309_WIDTH           128
#endif

#define OLED_RESET              0
#define OLED_WRITE_DATA         1
#define OLED_WRITE_COMMAND      2
#define OLED_DELAY              3

/* some LEDs don't display anything in first two columns  */
/* #define SSD1309_WIDTH           130			  */

/* Enumeration for screen colors			  */
typedef enum 
{
    Black = 0x00, /* Black color, no pixel		  */
    White = 0x01  /* Pixel is set. Color depends on OLED  */
} SSD1309_COLOR;

/* Struct to store transformations			  */
typedef struct 
{
    uint16_t CurrentX;
    uint16_t CurrentY;
    uint8_t Inverted;
    uint8_t Rotated;
    uint8_t Initialized;
} SSD1309_t;


/* Procedure definitions */
#if defined(SSD1309_USE_I2C)
void ssd1309_Init(ssd1309_i2c_handle i2c_comm_handle);
#elif defined(SSD1309_USE_SPI)
void ssd1309_Init(ssd1309_spi_handle spi_comm_handle);
#endif
void ssd1309_Fill(SSD1309_COLOR color);
void ssd1309_UpdateScreen(void);
bool ssd1309_Rotated(void);
void ssd1309_RotatedText(void);
void ssd1309_NonrotatedText(void);
void ssd1309_DrawPixel(uint8_t x, uint8_t y, SSD1309_COLOR color);
void ssd1309_WriteSymbol(SymbolID_t Symbol, uint8_t x, uint8_t y);
char ssd1309_WriteChar(char ch, FontDef Font, SSD1309_COLOR color);
char ssd1309_WriteString(char* str, FontDef Font, SSD1309_COLOR color);
void ssd1309_SetCursor(uint8_t x, uint8_t y);

/* Low-level procedures	*/
void ssd1309_Reset(void);
void ssd1309_WriteCommand(uint8_t byte);
void ssd1309_WriteData(uint8_t* buffer, size_t buff_size);

#endif /* __SSD1309_H__	*/
