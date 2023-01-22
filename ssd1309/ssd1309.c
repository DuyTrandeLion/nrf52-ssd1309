#include "ssd1309.h"

static float ssd1309_DegToRad(float par_deg);
static uint16_t ssd1309_NormalizeTo0_360(uint16_t par_deg);

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
static uint8_t SSD1309_Buffer[SSD1309_BUFFER_SIZE];

/* Screen object */
static SSD1309_t SSD1309;


/* Fills the Screenbuffer with values from a given buffer of a fixed length */
SSD1309_Error_t ssd1309_FillBuffer(uint8_t *buf, uint32_t len)
{
    SSD1309_Error_t ret = SSD1309_ERR;

    if (len <= SSD1309_BUFFER_SIZE)
    {
        memcpy(SSD1309_Buffer, buf, len);
        ret = SSD1309_OK;
    }

    return ret;
}


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

/* Set multiplex ratio. */
#if (SSD1309_HEIGHT == 128)
    /* Found in the Luma Python lib for SH1106. */
    ssd1306_WriteCommand(0xFF);
#else
    ssd1309_WriteCommand(0xA8); /*--set multiplex ratio(1 to 64) - CHECK */
#endif

#if (SSD1309_HEIGHT == 32)
    ssd1309_WriteCommand(0x1F);
#elif (SSD1309_HEIGHT == 64)
    ssd1309_WriteCommand(0x3F);
#elif (SSD1309_HEIGHT == 128)
    ssd1309_WriteCommand(0x3F); /* Seems to work for 128px high displays too. */
#else
#error "Only 32, 64, or 128 lines of height are supported!"
#endif

    ssd1309_WriteCommand(0xA4); /* 0xA4, Output follows RAM content;0xa5,Output ignores RAM content */

    ssd1309_WriteCommand(0xD3); /*-set display offset - CHECK */
    ssd1309_WriteCommand(0x00); /*-not offset */

    ssd1309_WriteCommand(0xD5); /*--set display clock divide ratio/oscillator frequency */
    ssd1309_WriteCommand(0xF0); /*--set divide ratio */

    ssd1309_WriteCommand(0xD9); /*--set pre-charge period */
    ssd1309_WriteCommand(0x22); /*			  */

    ssd1309_WriteCommand(0xDA); /*--set com pins hardware configuration - CHECK */
#if (SSD1309_HEIGHT == 32)
    ssd1309_WriteCommand(0x02);
#elif (SSD1309_HEIGHT == 64)
    ssd1309_WriteCommand(0x12);
#elif (SSD1309_HEIGHT == 128)
    ssd1309_WriteCommand(0x12);
#else
#error "Only 32, 64, or 128 lines of height are supported!"
#endif

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
    SSD1309.Rotation = ROTATION_0;
    
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
    /* Write data to each page of RAM. Number of pages
     * depends on the screen height:
     *
     *  * 32px   ==  4 pages
     *  * 64px   ==  8 pages
     *  * 128px  ==  16 pages 
     */
    for (uint8_t i = 0; i < (SSD1309_HEIGHT / 8); i++) 
    {
        ssd1309_WriteCommand(0xB0 + i);
        ssd1309_WriteCommand(0x00 + SSD1309_X_OFFSET_LOWER);
        ssd1309_WriteCommand(0x10 + SSD1309_X_OFFSET_UPPER);
        ssd1309_WriteData(&SSD1309_Buffer[SSD1309_WIDTH * i], SSD1309_WIDTH);
    }
}

/*    Draw one pixel in the screenbuffer  */
/*    X => X Coordinate			  */
/*    Y => Y Coordinate			  */
/*    color => Pixel color		  */
void ssd1309_DrawPixel(uint8_t x, uint8_t y, SSD1309_COLOR color) 
{
    if ((x >= SSD1309_WIDTH) || (y >= SSD1309_HEIGHT)) 
    {
        /* Don't write outside the buffer */
        return;
    }
    
    /* Draw in the right color */
    if (color == White) 
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

    /* Check if character is valid */
    if (ch < 32 || ch > 126)
    {
        return 0;
    }
    
    /* Check remaining space on current line */
    if ((SSD1309_WIDTH <= (SSD1309.CurrentX + Font.FontWidth))  ||
        (SSD1309_HEIGHT <= (SSD1309.CurrentY + Font.FontHeight))
       )
    {
        /* Not enough space on current line */
        return 0;
    }
    
    /* Use the font to write */
    for (i = 0; i < Font.FontHeight; i++) 
    {
	b = Font.data[(ch - 32) * Font.FontHeight + i];

        for (j = 0; j < Font.FontWidth; j++) 
	{
            if ((b << j) & 0x8000)  
	    {
                ssd1309_DrawPixel(SSD1309.CurrentX + j, (SSD1309.CurrentY + i), (SSD1309_COLOR)color);
            } 
	    else 
	    {
                ssd1309_DrawPixel(SSD1309.CurrentX + j, (SSD1309.CurrentY + i), (SSD1309_COLOR)!color);
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
    if ((SSD1309_WIDTH <= (SSD1309.CurrentX + SSD1309_Symbol[Symbol].SymbolWidth))  ||
        (SSD1309_HEIGHT <= (SSD1309.CurrentY + SSD1309_Symbol[Symbol].SymbolHeight))
       )
    {
        /* Not enough space on current line */
        return ;
    }

    /* Use the data to write */
    for (i = 0; i <= SSD1309_Symbol[Symbol].SymbolHeight; i++) 
    {
	b = SSD1309_Symbol[Symbol].data[i];

        for (j = 0; j < SSD1309_Symbol[Symbol].SymbolWidth; j++) 
	{
            if ((b << j) & 0x8000)  
	    {
                ssd1309_DrawPixel(SSD1309.CurrentX + j, (SSD1309.CurrentY + i), (SSD1309_COLOR)color);
            } 
	    else 
	    {
                ssd1309_DrawPixel(SSD1309.CurrentX + j, (SSD1309.CurrentY + i), (SSD1309_COLOR)!color);
            }
        }
    }
    
    /* The current space is now taken */
    SSD1309.CurrentX += SSD1309_Symbol[Symbol].SymbolWidth;
}


/* Write full string to screenbuffer */
char ssd1309_WriteString(char* str, FontDef Font, SSD1309_COLOR color) 
{
    char data[64];
    uint8_t data_counter = 0;
    uint8_t print_counter;

    CLEAR_BUFFER(data);

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
    
    /* Everything ok */
    return *str;
}


/* Position the cursor */
void ssd1309_SetCursor(uint8_t x, uint8_t y) 
{
    SSD1309.CurrentX = x - SSD1309_OFFSET_X;
    SSD1309.CurrentY = y - SSD1309_OFFSET_Y;
}


/* Draw line by Bresenhem's algorithm */
void ssd1309_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SSD1309_COLOR color)
{
    int32_t deltaX = abs(x2 - x1);
    int32_t deltaY = abs(y2 - y1);
    int32_t signX = ((x1 < x2) ? 1 : -1);
    int32_t signY = ((y1 < y2) ? 1 : -1);
    int32_t error = deltaX - deltaY;
    int32_t error2;

    ssd1309_DrawPixel(x2, y2, color);
    while ((x1 != x2) || (y1 != y2))
    {
        ssd1309_DrawPixel(x1, y1, color);
        error2 = error * 2;

        if (error2 > -deltaY)
        {
            error -= deltaY;
            x1 += signX;
        }
        else
        {
            /* Nothing to do */    
        }
    
        if (error2 < deltaX)
        {
            error += deltaX;
            y1 += signY;
        }
        else
        {
            /* Nothing to do */
        }
    }
    return;
}


/* DrawArc. Draw angle is beginning from 4 quart of trigonometric circle (3pi/2)
 * start_angle in degree
 * sweep in degree
 */
void ssd1309_DrawArc(uint8_t x, uint8_t y, uint8_t radius, uint16_t start_angle, uint16_t sweep, SSD1309_COLOR color)
{
    #define CIRCLE_APPROXIMATION_SEGMENTS 36
    float approx_degree;
    uint32_t approx_segments;
    uint8_t xp1,xp2;
    uint8_t yp1,yp2;
    uint16_t loc_sweep = 0;
    uint16_t loc_angle_count = 0;
    uint32_t count = 0;
    
    float rad;
    
      loc_sweep = ssd1309_NormalizeTo0_360(sweep);
      loc_angle_count = ssd1309_NormalizeTo0_360(start_angle);
    
    count = (loc_angle_count * CIRCLE_APPROXIMATION_SEGMENTS) / 360;
    approx_segments = (loc_sweep * CIRCLE_APPROXIMATION_SEGMENTS) / 360;
    approx_degree = loc_sweep / (float)approx_segments;

    while (count < approx_segments)
    {
        xp1 = x + (int8_t)(sin(rad) * radius);
        yp1 = y + (int8_t)(cos(rad) * radius);    
        count++;
#if 1
        if (count != approx_segments)
        {
              rad = ssd1309_DegToRad(count * approx_degree);
        }
        else
        {            
            rad = ssd1309_DegToRad(loc_sweep);
        }
#endif
        xp2 = x + (int8_t)(sin(rad) * radius);
        yp2 = y + (int8_t)(cos(rad) * radius);    
        ssd1309_DrawLine(xp1, yp1, xp2, yp2, color);
    }
}


/*
 * Draw arc with radius line
 * Angle is beginning from 4 quart of trigonometric circle (3pi/2)
 * start_angle: start angle in degree
 * sweep: finish angle in degree
 */
void ssd1309_DrawArcWithRadiusLine(uint8_t x, uint8_t y, uint8_t radius, uint16_t start_angle, uint16_t sweep, SSD1309_COLOR color)
{
    static const uint8_t CIRCLE_APPROXIMATION_SEGMENTS = 36;
    float approx_degree;
    uint32_t approx_segments;
    uint8_t xp1 = 0;
    uint8_t xp2 = 0;
    uint8_t yp1 = 0;
    uint8_t yp2 = 0;
    uint32_t count = 0;
    uint32_t loc_sweep = 0;
    float rad;
    
    loc_sweep = ssd1309_NormalizeTo0_360(sweep);
    
    count = (ssd1309_NormalizeTo0_360(start_angle) * CIRCLE_APPROXIMATION_SEGMENTS) / 360;
    approx_segments = (loc_sweep * CIRCLE_APPROXIMATION_SEGMENTS) / 360;
    approx_degree = loc_sweep / (float)approx_segments;

    rad = ssd1309_DegToRad(count * approx_degree);
    uint8_t first_point_x = x + (int8_t)(sin(rad)*radius);
    uint8_t first_point_y = y + (int8_t)(cos(rad)*radius);

    while (count < approx_segments)
    {
        rad = ssd1309_DegToRad(count*approx_degree);
        xp1 = x + (int8_t)(sin(rad)*radius);
        yp1 = y + (int8_t)(cos(rad)*radius);    
        count++;

        if (count != approx_segments)
        {
            rad = ssd1309_DegToRad(count*approx_degree);
        }
        else
        {            
            rad = ssd1309_DegToRad(loc_sweep);
        }

        xp2 = x + (int8_t)(sin(rad)*radius);
        yp2 = y + (int8_t)(cos(rad)*radius);    
        ssd1309_DrawLine(xp1, yp1, xp2, yp2, color);
    }
    
    // Radius line
    ssd1309_DrawLine(x, y, first_point_x, first_point_y, color);
    ssd1309_DrawLine(x, y, xp2, yp2, color);
}


/* Draw circle by Bresenhem's algorithm */
void ssd1309_DrawCircle(uint8_t par_x, uint8_t par_y, uint8_t par_r, SSD1309_COLOR color)
{
    int32_t x = -par_r;
    int32_t y = 0;
    int32_t err = 2 - 2 * par_r;
    int32_t e2;

    if (par_x >= SSD1309_WIDTH || par_y >= SSD1309_HEIGHT)
    {
        return;
    }

    do {
        ssd1309_DrawPixel(par_x - x, par_y + y, color);
        ssd1309_DrawPixel(par_x + x, par_y + y, color);
        ssd1309_DrawPixel(par_x + x, par_y - y, color);
        ssd1309_DrawPixel(par_x - x, par_y - y, color);
        e2 = err;

        if (e2 <= y)
        {
            y++;
            err = err + (y * 2 + 1);

            if (-x == y && e2 <= x)
            {
                e2 = 0;
            }
            else
            {
                /* Nothing to do */
            }
        }
        else
        {
            /* Nothing to do */
        }

        if (e2 > x)
        {
            x++;
            err = err + (x * 2 + 1);
        }
        else
        {
            /* Nothing to do */
        }
    } while (x <= 0);

    return;
}

/* Draw filled circle. Pixel positions calculated using Bresenham's algorithm */
void ssd1309_FillCircle(uint8_t par_x,uint8_t par_y,uint8_t par_r, SSD1309_COLOR par_color)
{
    int32_t x = -par_r;
    int32_t y = 0;
    int32_t err = 2 - 2 * par_r;
    int32_t e2;

    if (par_x >= SSD1306_WIDTH || par_y >= SSD1306_HEIGHT) {
        return;
    }

    do {
        for (uint8_t _y = (par_y + y); _y >= (par_y - y); _y--)
        {
            for (uint8_t _x = (par_x - x); _x >= (par_x + x); _x--)
            {
                ssd1309_DrawPixel(_x, _y, par_color);
            }
        }

        e2 = err;
        if (e2 <= y)
        {
            y++;
            err = err + (y * 2 + 1);
            if (-x == y && e2 <= x) {
                e2 = 0;
            }
            else {
                /* nothing to do */
            }
        }
        else
        {
            /* nothing to do */
        }

        if (e2 > x)
        {
            x++;
            err = err + (x * 2 + 1);
        }
        else
        {
            /* nothing to do */
        }
    } while (x <= 0);
}


/* Draw polyline */
void ssd1309_Polyline(const SSD1309_VERTEX *par_vertex, uint16_t par_size, SSD1309_COLOR color)
{
    uint16_t i;

    if (par_vertex != NULL)
    {
        for (i = 1; i < par_size; i++)
        {
            ssd1309_DrawLine(par_vertex[i - 1].x, par_vertex[i - 1].y, par_vertex[i].x, par_vertex[i].y, color);
        }
    }
    else
    {
        /* Nothing to do */
    }
    return;
}


/* Draw rectangle */
void ssd1309_DrawRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SSD1309_COLOR color)
{
    ssd1309_DrawLine(x1, y1, x2, y1, color);
    ssd1309_DrawLine(x2, y1, x2, y2, color);
    ssd1309_DrawLine(x2, y2, x1, y2, color);
    ssd1309_DrawLine(x1, y2, x1, y1, color);
}


/* Draw filled rectangle */
void ssd1309_FillRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SSD1309_COLOR color)
{
    uint8_t x_start = ((x1 <= x2) ? x1 : x2);
    uint8_t x_end   = ((x1 <= x2) ? x2 : x1);
    uint8_t y_start = ((y1 <= y2) ? y1 : y2);
    uint8_t y_end   = ((y1 <= y2) ? y2 : y1);

    for (uint8_t y = y_start; (y <= y_end) && (y < SSD1309_HEIGHT); y++) {
        for (uint8_t x = x_start; (x <= x_end) && (x < SSD1309_WIDTH); x++) {
            ssd1309_DrawPixel(x, y, color);
        }
    }
    return;
}


/* Draw bitmap - ported from the ADAFruit GFX library */
void ssd1309_DrawBitmap(uint8_t x, uint8_t y, const unsigned char* bitmap, uint8_t w, uint8_t h, SSD1309_COLOR color)
{
    uint8_t byte = 0;
    int16_t byteWidth = (w + 7) / 8; /* Bitmap scanline pad = whole byte */

    if (x >= SSD1309_WIDTH || y >= SSD1309_HEIGHT)
    {
        return;
    }

    for (uint8_t j = 0; j < h; j++, y++)
    {
        for (uint8_t i = 0; i < w; i++)
        {
            if (i & 7)
            {
                byte <<= 1;
            }
            else
            {
                byte = (*(const unsigned char *)(&bitmap[j * byteWidth + i / 8]));
            }

            if (byte & 0x80)
            {
                ssd1309_DrawPixel(x + i, y, color);
            }
        }
    }

    return;
}


void ssd1309_SetContrast(const uint8_t value)
{
    const uint8_t kSetContrastControlRegister = 0x81;

    ssd1309_WriteCommand(kSetContrastControlRegister);
    ssd1309_WriteCommand(value);
}


/* Convert Degrees to Radians */
static float ssd1309_DegToRad(float par_deg) {
    return par_deg * 3.14 / 180.0;
}

/* Normalize degree to [0; 360] */
static uint16_t ssd1309_NormalizeTo0_360(uint16_t par_deg) {
  uint16_t loc_angle;
  if(par_deg <= 360)
  {
    loc_angle = par_deg;
  }
  else
  {
    loc_angle = par_deg % 360;
    loc_angle = ((par_deg != 0)?par_deg:360);
  }
  return loc_angle;
}