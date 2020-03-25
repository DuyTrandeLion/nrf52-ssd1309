#include "ssd1309.h"
#include "Miscellaneous.h"


#if defined(SSD1309_USE_I2C)
ssd1309_i2c_handle i2c_comm_handle_callback;

void ssd1309_Reset(void) 
{
    /* for I2C - do nothing */
}

/* Send a byte to the command register */
void ssd1309_WriteCommand(uint8_t byte) 
{
    tx_buff tx_buff[3] = {0x00, 0x00, 0x00};
    buff[2] = byte;

    if (NULL != i2c_comm_handle_callback)
    {
        i2c_comm_handle_callback(SSD1309_I2C_ADDR, tx_buff, MAX_ADDRESS_SIZE + 1);
    }
}

/* Send datam*/
void ssd1309_WriteData(uint8_t* buffer, size_t buff_size) 
{
    uint8_t tx_buff[MAX_TX_SIZE];
    memset(tx_buff, 0, MAX_TX_SIZE);

    if (buff_size > MAX_TX_DATA)
    {
	return;
    }

    memcpy(&tx_buff[2], buffer, buff_size);

    if (NULL != i2c_comm_handle_callback)
    {
        i2c_comm_handle_callback(SSD1309_I2C_ADDR, tx_buff, MAX_ADDRESS_SIZE + buff_size);
    }
}

#elif defined(SSD1309_USE_SPI)
ssd1309_spi_handle spi_comm_handle_callback;

void ssd1309_Reset(void) 
{
    if (NULL != spi_comm_handle_callback)
    {
        spi_comm_handle_callback(OLED_RESET, NULL, 0);
    }
}

/* Send a byte to the command register */
void ssd1309_WriteCommand(uint8_t byte) 
{
    if (NULL != spi_comm_handle_callback)
    {
        spi_comm_handle_callback(OLED_WRITE_COMMAND, (uint8_t *)&byte, 1);
    }
}

/* Send data */
void ssd1309_WriteData(uint8_t *buffer, size_t buff_size) 
{
    if (NULL != spi_comm_handle_callback)
    {
        spi_comm_handle_callback(OLED_WRITE_DATA, buffer, buff_size);
    }
}

#else
#error "You should define SSD1309_USE_SPI or SSD1309_USE_I2C macro"
#endif

/* Screenbuffer */
static uint8_t SSD1309_Buffer[SSD1309_WIDTH * SSD1309_HEIGHT / 8];

/* Screen object */
static SSD1309_t SSD1309;


/* Initialize the oled screen */
#if defined(SSD1309_USE_I2C)
void ssd1309_Init(ssd1309_i2c_handle i2c_comm_handle) 
{
    if (NULL != i2c_comm_handle)
    {
        i2c_comm_handle_callback = i2c_comm_handle;
    }
#elif defined(SSD1309_USE_SPI)
void ssd1309_Init(ssd1309_spi_handle spi_comm_handle) 
{
    uint8_t ssd1309_DelayTimeMS = 100;
    if (NULL != spi_comm_handle)
    {
        spi_comm_handle_callback = spi_comm_handle;
    }
#endif
    /* Reset OLED */
    ssd1309_Reset();

    /* Wait for the screen to boot */
    spi_comm_handle_callback(OLED_DELAY, &ssd1309_DelayTimeMS, sizeof(uint8_t));
    
    /* Init OLED */
    ssd1309_WriteCommand(0xAE); /* Display off */

    ssd1309_WriteCommand(0x20); /* Set Memory Addressing Mode */   
    ssd1309_WriteCommand(0x10); /* 00,Horizontal Addressing Mode; 01,Vertical Addressing Mode; */
                                /* 10,Page Addressing Mode (RESET); 11,Invalid */

    ssd1309_WriteCommand(0xB0); /*Set Page Start Address for Page Addressing Mode, 0-7 */

#ifdef SSD1309_MIRROR_VERT
    ssd1309_WriteCommand(0xC0); /* Mirror vertically */
#else
    ssd1309_WriteCommand(0xC8); /* Set COM Output Scan Direction */
#endif

    ssd1309_WriteCommand(0x00); /*---set low column address  */
    ssd1309_WriteCommand(0x10); /*---set high column address */

    ssd1309_WriteCommand(0x40); /*--set start line address - CHECK */

    ssd1309_WriteCommand(0x81); /*--set contrast control register - CHECK */
    ssd1309_WriteCommand(0xFF);

#ifdef SSD1309_MIRROR_HORIZ
    ssd1309_WriteCommand(0xA0); /* Mirror horizontally */
#else
    ssd1309_WriteCommand(0xA1); /* --set segment re-map 0 to 127 - CHECK */
#endif

#ifdef SSD1309_INVERSE_COLOR
    ssd1309_WriteCommand(0xA7); /*--set inverse color */
#else
    ssd1309_WriteCommand(0xA6); /*--set normal color */
#endif

    ssd1309_WriteCommand(0xA8); /*--set multiplex ratio(1 to 64) - CHECK  */
    ssd1309_WriteCommand(0x3F); /*                                        */

    ssd1309_WriteCommand(0xA4); /* 0xA4,Output follows RAM content;0xa5,Output ignores RAM content */

    ssd1309_WriteCommand(0xD3); /*-set display offset - CHECK */
    ssd1309_WriteCommand(0x00); /*-not offset */

    ssd1309_WriteCommand(0xD5); /*--set display clock divide ratio/oscillator frequency */
    ssd1309_WriteCommand(0xF0); /*--set divide ratio */

    ssd1309_WriteCommand(0xD9); /*--set pre-charge period */
    ssd1309_WriteCommand(0x22); /*			  */

    ssd1309_WriteCommand(0xDA); /*--set com pins hardware configuration - CHECK */
    ssd1309_WriteCommand(0x12);

    ssd1309_WriteCommand(0xDB); /*--set vcomh */
    ssd1309_WriteCommand(0x20); /* 0x20, 0.77xVcc */ 

    ssd1309_WriteCommand(0x8D); /*--set DC-DC enable */
    ssd1309_WriteCommand(0x14); /*                   */
    ssd1309_WriteCommand(0xAF); /*--turn on SSD1309 panel */

    /* Clear screen */
    ssd1309_Fill(Black);
    
    /* Flush buffer to screen */
    ssd1309_UpdateScreen();
    
    /* Set default values for screen object */
    SSD1309.CurrentX = 0;
    SSD1309.CurrentY = 0;
    SSD1309.Rotated  = 0;
    
    SSD1309.Initialized = 1; 
}

/* Fill the whole screen with the given color */
void ssd1309_Fill(SSD1309_COLOR color) 
{
    /* Set memory */
    uint32_t i;

    for(i = 0; i < sizeof(SSD1309_Buffer); i++) 
    {
        SSD1309_Buffer[i] = (color == Black) ? 0x00 : 0xFF;
    }
}

/* Write the screenbuffer with changed to the screen */
void ssd1309_UpdateScreen(void) 
{
    uint8_t i;
    for(i = 0; i < 8; i++) 
    {
        ssd1309_WriteCommand(0xB0 + i);
        ssd1309_WriteCommand(0x00);
        ssd1309_WriteCommand(0x10);
        ssd1309_WriteData(&SSD1309_Buffer[SSD1309_WIDTH*i],SSD1309_WIDTH);
    }
}

/*    Draw one pixel in the screenbuffer  */
/*    X => X Coordinate			  */
/*    Y => Y Coordinate			  */
/*    color => Pixel color		  */
void ssd1309_DrawPixel(uint8_t x, uint8_t y, SSD1309_COLOR color) 
{
    if((x >= SSD1309_WIDTH) || (y >= SSD1309_HEIGHT)) 
    {
        /* Don't write outside the buffer */
        return;
    }
    
    /* Check if pixel should be inverted */
    if(SSD1309.Inverted) 
    {
        color = (SSD1309_COLOR)!color;
    }
    
    /* Draw in the right color */
    if(color == White) 
    {
	SSD1309_Buffer[x + (y / 8) * SSD1309_WIDTH] |= 1 << (y % 8);
    } 
    else 
    { 
	SSD1309_Buffer[x + (y / 8) * SSD1309_WIDTH] &= ~(1 << (y % 8));
    }
}

/* Draw 1 char to the screen buffer	      */
/* ch         => char om weg te schrijven     */
/* Font     => Font waarmee we gaan schrijven */
/* color     => Black or White                */
char ssd1309_WriteChar(char ch, FontDef Font, SSD1309_COLOR color) 
{
    uint32_t i, b, j;
    
    /* Check remaining space on current line */
    if (0 == SSD1309.Rotated)
    {
	if ((SSD1309_WIDTH <= (SSD1309.CurrentX + Font.FontWidth))  ||
	    (SSD1309_HEIGHT <= (SSD1309.CurrentY + Font.FontHeight))
	   )
	{
	    /* Not enough space on current line */
	    return 0;
	}
    }
    else
    {
	if ((SSD1309_WIDTH <= (SSD1309.CurrentX - Font.FontWidth))  ||
	    (SSD1309_HEIGHT <= (SSD1309.CurrentY - Font.FontHeight))
	   )
	{
	    /* Not enough space on current line */
	    return 0;
	}
    }
    
    /* Use the font to write */
    for (i = 0; i < Font.FontHeight; i++) 
    {
	b = Font.data[(ch - 32) * Font.FontHeight + i];

        for (j = 0; j < Font.FontWidth; j++) 
	{
            if ((b << j) & 0x8000)  
	    {
		if (0 == SSD1309.Rotated)
		{
		    ssd1309_DrawPixel(SSD1309.CurrentX + j, (SSD1309.CurrentY + i), (SSD1309_COLOR)color);
		}
		else
		{
                    ssd1309_DrawPixel(SSD1309.CurrentX - j, (SSD1309.CurrentY - i), (SSD1309_COLOR)color);
		}
            } 
	    else 
	    {
		if (0 == SSD1309.Rotated)
		{
		    ssd1309_DrawPixel(SSD1309.CurrentX + j, (SSD1309.CurrentY + i), (SSD1309_COLOR)!color);
                }
		else
		{
                    ssd1309_DrawPixel(SSD1309.CurrentX - j, (SSD1309.CurrentY - i), (SSD1309_COLOR)!color);
		}
            }
        }
    }
    
    /* The current space is now taken */
    SSD1309.CurrentX += Font.FontWidth;
    
    /* Return written char for validation */
    return ch;
}


void ssd1309_WriteSymbol(SymbolID_t Symbol, uint8_t x, uint8_t y)
{
    uint32_t i, b, j;
    SSD1309_COLOR color = White;

    ssd1309_SetCursor(x, y);
    
    /* Check remaining space on current line */
    if (0 == SSD1309.Rotated)
    {
	if ((SSD1309_WIDTH <= (SSD1309.CurrentX + SSD1309_Symbol[Symbol].SymbolWidth))  ||
	    (SSD1309_HEIGHT <= (SSD1309.CurrentY + SSD1309_Symbol[Symbol].SymbolHeight))
	   )
	{
	    /* Not enough space on current line */
	    return ;
	}
    }
    else
    {
	if ((SSD1309_WIDTH <= (SSD1309.CurrentX - SSD1309_Symbol[Symbol].SymbolWidth))  ||
	    (SSD1309_HEIGHT <= (SSD1309.CurrentY - SSD1309_Symbol[Symbol].SymbolHeight))
	   )
	{
	    /* Not enough space on current line */
	    return ;
	}
    }

    /* Use the data to write */
    for (i = 0; i <= SSD1309_Symbol[Symbol].SymbolHeight; i++) 
    {
	b = SSD1309_Symbol[Symbol].data[i];

        for (j = 0; j < SSD1309_Symbol[Symbol].SymbolWidth; j++) 
	{
            if ((b << j) & 0x8000)  
	    {
		if (0 == SSD1309.Rotated)
		{
		    ssd1309_DrawPixel(SSD1309.CurrentX + j, (SSD1309.CurrentY + i), (SSD1309_COLOR)color);
		}
		else
		{
                    ssd1309_DrawPixel(SSD1309.CurrentX - j, (SSD1309.CurrentY - i), (SSD1309_COLOR)color);
		}
            } 
	    else 
	    {
		if (0 == SSD1309.Rotated)
		{
		    ssd1309_DrawPixel(SSD1309.CurrentX + j, (SSD1309.CurrentY + i), (SSD1309_COLOR)!color);
                }
		else
		{
                    ssd1309_DrawPixel(SSD1309.CurrentX - j, (SSD1309.CurrentY - i), (SSD1309_COLOR)!color);
		}
            }
        }
    }
    
    /* The current space is now taken */
    SSD1309.CurrentX += SSD1309_Symbol[Symbol].SymbolWidth;
}


void reverseString(char* str) 
{ 
    int l, i; 
    char *begin_ptr, *end_ptr, ch; 
  
    // Get the length of the string 
    l = strlen(str); 
  
    // Set the begin_ptr and end_ptr 
    // initially to start of string 
    begin_ptr = str; 
    end_ptr = str; 
  
    // Move the end_ptr to the last character 
    for (i = 0; i < l - 1; i++) 
        end_ptr++; 
  
    // Swap the char from start and end 
    // index using begin_ptr and end_ptr 
    for (i = 0; i < l / 2; i++) { 
  
        // swap character 
        ch = *end_ptr; 
        *end_ptr = *begin_ptr; 
        *begin_ptr = ch; 
  
        // update pointers positions 
        begin_ptr++; 
        end_ptr--; 
    } 
}

/* Write full string to screenbuffer */
char ssd1309_WriteString(char* str, FontDef Font, SSD1309_COLOR color) 
{
    char data[64];
    uint8_t data_counter = 0;
    uint8_t print_counter;

    CLEAR_BUFFER(data);

    if (0 != SSD1309.Rotated)
    {
	while (*str)
	{
	    data[data_counter] = *str;
	    data_counter++;
            str++;
	}
    }

    if (0 == SSD1309.Rotated)
    {
	/* Write until null-byte */
	while (*str) 
	{
	    if (ssd1309_WriteChar(*str, Font, color) != *str) 
	    {
		/* Char could not be written */
		return *str;
	    }
        
	    /* Next char */
	    str++;
	}
    }
    else
    {
	for (print_counter = 0; print_counter < data_counter; print_counter++)
	{
	    if (ssd1309_WriteChar(data[(data_counter - 1) - print_counter], Font, color) != data[(data_counter - 1) - print_counter]) 
	    {
		/* Char could not be written */
		return data[(data_counter - 1) - print_counter];
	    }
	}
    }
    
    /* Everything ok */
    return *str;
}

/* Position the cursor */
void ssd1309_SetCursor(uint8_t x, uint8_t y) 
{
    if (0 == SSD1309.Rotated)
    {
	SSD1309.CurrentX = x;
	SSD1309.CurrentY = y;
    }
    else
    {
	SSD1309.CurrentX = SSD1309_WIDTH - x;
	SSD1309.CurrentY = SSD1309_HEIGHT - 1 - y;
    }
}


bool ssd1309_Rotated(void)
{
    if (0 == SSD1309.Rotated)
    {
	return false;
    }

    return true;
}


void ssd1309_RotatedText(void)
{
    SSD1309.Rotated = 1;
}


void ssd1309_NonrotatedText(void)
{
    SSD1309.Rotated = 0;
}