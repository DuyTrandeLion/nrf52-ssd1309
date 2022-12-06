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

#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "ssd1309_conf.h"
#include "ssd1309_fonts.h"

#include "nordic_common.h"
#include "nrf.h"
#include "boards.h"
#include "nrf_gpio.h"
#include "app_util_platform.h"
#include "app_error.h"

#if defined(SSD1309_USE_I2C)
#ifndef SSD1309_I2C_ADDR
#define SSD1309_I2C_ADDR        0x3C
#endif

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
#define SSD1309_WIDTH           130
#endif

/* SSD1309 offset of x in pixels  */
#ifndef SSD1309_OFFSET_X
#define SSD1309_OFFSET_X        -2
#define SSD1309_X_OFFSET_LOWER (SSD1309_X_OFFSET & 0x0F)
#define SSD1309_X_OFFSET_UPPER ((SSD1309_X_OFFSET >> 4) & 0x07)
#else
#define SSD1309_X_OFFSET_LOWER 0
#define SSD1309_X_OFFSET_UPPER 0
#endif

/* SSD1309 offset of y in pixels  */
#ifndef SSD1309_OFFSET_Y
#define SSD1309_OFFSET_Y        0
#endif

#define SSD1309_BUFFER_SIZE     (SSD1309_WIDTH * SSD1309_HEIGHT / 8)

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

typedef enum
{
    SSD1309_OK  = 0x00,
    SSD1309_ERR = 0x01  /* Generic error                  */
} SSD1309_Error_t;

typedef enum
{
    ROTATION_0    = 0,
    ROTATION_90   = 1,
    ROTATION_180  = 2,
    ROTATION_270  = 3
} SSD1309_ROTATION;

/* Struct to store transformations			  */
typedef struct 
{
    uint16_t CurrentX;
    uint16_t CurrentY;
    SSD1309_ROTATION Rotation;
    uint8_t Initialized;
    uint8_t DisplayOn;
} SSD1309_t;

typedef struct
{
    uint8_t x;
    uint8_t y;
} SSD1309_VERTEX;


/* Procedure definitions */
#if defined(SSD1309_USE_I2C)
void ssd1309_Init(ssd1309_i2c_handle i2c_comm_handle);
#elif defined(SSD1309_USE_SPI)
void ssd1309_Init(ssd1309_spi_handle spi_comm_handle);
#endif
void ssd1309_SetContrast(const uint8_t value);

void ssd1309_Fill(SSD1309_COLOR color);
void ssd1309_UpdateScreen(void);
void ssd1309_DrawPixel(uint8_t x, uint8_t y, SSD1309_COLOR color);
void ssd1309_WriteSymbol(SymbolID_t Symbol, uint8_t x, uint8_t y);
char ssd1309_WriteChar(char ch, FontDef Font, SSD1309_COLOR color);
char ssd1309_WriteString(char* str, FontDef Font, SSD1309_COLOR color);
void ssd1309_SetCursor(uint8_t x, uint8_t y);
void ssd1309_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SSD1309_COLOR color);
void ssd1309_DrawArc(uint8_t x, uint8_t y, uint8_t radius, uint16_t start_angle, uint16_t sweep, SSD1309_COLOR color);
void ssd1309_DrawArcWithRadiusLine(uint8_t x, uint8_t y, uint8_t radius, uint16_t start_angle, uint16_t sweep, SSD1309_COLOR color);
void ssd1309_DrawCircle(uint8_t par_x, uint8_t par_y, uint8_t par_r, SSD1309_COLOR color);
void ssd1309_FillCircle(uint8_t par_x,uint8_t par_y, uint8_t par_r, SSD1309_COLOR par_color);
void ssd1309_Polyline(const SSD1309_VERTEX *par_vertex, uint16_t par_size, SSD1309_COLOR color);
void ssd1309_DrawRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SSD1309_COLOR color);
void ssd1309_FillRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SSD1309_COLOR color);
void ssd1309_DrawBitmap(uint8_t x, uint8_t y, const unsigned char* bitmap, uint8_t w, uint8_t h, SSD1309_COLOR color);

/**
 * @brief Sets the contrast of the display.
 * @param[in] value contrast to set.
 * @note Contrast increases as the value increases.
 * @note RESET = 7Fh.
 */
void ssd1309_SetContrast(const uint8_t value);

/* Low-level procedures	*/
void ssd1309_Reset(void);
void ssd1309_WriteCommand(uint8_t byte);
void ssd1309_WriteData(uint8_t* buffer, size_t buff_size);
SSD1309_Error_t ssd1309_FillBuffer(uint8_t *buf, uint32_t len);

#endif /* __SSD1309_H__	*/
