#include <stdint.h>

#ifndef __SSD1309_FONTS_H__
#define __SSD1309_FONTS_H__

#define FONT_7x10_ID      0x00
#define FONT_11x18_ID     0x01
#define FONT_16x26_ID     0x02
#define SYMBOL_7x10_ID    0x03
#define SYMBOL_11x18_ID   0x04

typedef enum
{
    POWER,
    ENTER,
    ANTENNA,
    BATTERY_0,
    BATTERY_33,
    BATTERY_66,
    BATTERY_100,
    BLE,
    DOWN,
    UP,
    ALL_SYMBOL,
} SymbolID_t;


typedef struct
{
  const uint8_t SymbolID;
  const uint8_t SymbolWidth;
  uint8_t SymbolHeight;
  const uint16_t *data;
} SymbolDef;

typedef struct 
{
	const uint8_t FontID;
	const uint8_t FontWidth;    /*!< Font width in pixels */
	uint8_t FontHeight;	    /*!< Font height in pixels */
	const uint16_t *data;	    /*!< Pointer to data font data array */
} FontDef;

extern FontDef Font_7x10;
extern FontDef Font_11x18;
extern FontDef Font_16x26;

extern SymbolDef SSD1309_Symbol[];

#endif /* __SSD1309_FONTS_H__ */
