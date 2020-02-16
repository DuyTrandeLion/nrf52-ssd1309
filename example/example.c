
/* Define GPIOs before using */
#define GPIO_PIN_SET		1
#define GPIO_PIN_RESET		0

#define OLED_SCK_PIN	      NRF_GPIO_PIN_MAP(1, 15)
#define OLED_MOSI_PIN	      NRF_GPIO_PIN_MAP(1, 13)
#define OLED_SS_PIN	      	  NRF_GPIO_PIN_MAP(1, 12)
#define OLED_ENABLE_PIN       NRF_GPIO_PIN_MAP(1, 2)
#define OLED_DC_PIN           NRF_GPIO_PIN_MAP(1, 11)
#define OLED_RES_PIN          NRF_GPIO_PIN_MAP(1, 10)

/* Define macros for easier usage */
#define oled_enable()			nrf_gpio_pin_write(OLED_ENABLE_PIN, GPIO_PIN_SET)
#define oled_disable()			nrf_gpio_pin_write(OLED_ENABLE_PIN, GPIO_PIN_RESET)

#define oled_write_string(string, x, y) if (false == ssd1309_Rotated())			      \
					{						      \
					    ssd1309_SetCursor(x, y);			      \
					}						      \
					else						      \
					{						      \
                                            ssd1309_SetCursor(x + (7 * strlen(string)), y);   \
					}						      \
                                        ssd1309_WriteString(string, Font_7x10, White)

#define oled_write_mstring(string, x, y) if (false == ssd1309_Rotated())                      \
					{						      \
					    ssd1309_SetCursor(x, y);			      \
					}						      \
					else						      \
					{						      \
                                            ssd1309_SetCursor(x + (11 * strlen(string)), y);  \
					}						      \
                                        ssd1309_WriteString(string, Font_11x18, White)

#define oled_write_symbol(symbol, x, y) ssd1309_WriteSymbol(symbol, x, y)

#define oled_update_screen()		ssd1309_UpdateScreen()
#define oled_clear_screen()		ssd1309_Fill(Black)

typedef enum
{
    OLED_SPI_READY = 0,
    OLED_SPI_BUSY,
    OLED_SPI_NOT_AVAILABLE
} oled_spi_state_t;

/* SPI instances */
const nrf_drv_spi_t m_oled_spi   = NRF_DRV_SPI_INSTANCE(OLED_SPI_INSTANCE);

#if defined(SSD1309_USE_I2C)
void i2c_oled_comm_handle(uint8_t hdl_address, uint8_t *hdl_buffer, size_t hdl_buffer_size)
{
    nrf_drv_twi_tx(&m_oled_twi, hdl_address, hdl_buffer, hdl_buffer_size, false);
}
#elif defined(SSD1309_USE_SPI)
void spi_oled_comm_handle(uint8_t hdl_type, uint8_t *hdl_buffer, size_t hdl_buffer_size)
{
    switch (hdl_type)
    {
        case OLED_RESET:
        {
            /* CS = High (not selected) */
            nrf_gpio_pin_write(OLED_SS_PIN, GPIO_PIN_SET);

            /* Reset the OLED */
            nrf_gpio_pin_write(OLED_RES_PIN, GPIO_PIN_RESET);
            nrf_delay_ms(10);
            nrf_gpio_pin_write(OLED_RES_PIN, GPIO_PIN_SET);
            nrf_delay_ms(10);
            break;
        }

        case OLED_WRITE_COMMAND:
        {
            nrf_gpio_pin_write(OLED_DC_PIN, GPIO_PIN_RESET); 
            APP_ERROR_CHECK(nrf_drv_spi_transfer(&m_oled_spi, hdl_buffer, hdl_buffer_size, NULL, NULL));
            break;
        }

        case OLED_WRITE_DATA:
        {
            nrf_gpio_pin_write(OLED_DC_PIN, GPIO_PIN_SET); 
            APP_ERROR_CHECK(nrf_drv_spi_transfer(&m_oled_spi, hdl_buffer, hdl_buffer_size, NULL, NULL));
            break;
        }

        default: break;
    }
}
#endif

/**
 * @brief SPI initialization.
 */
void spi_init(void)
{
#if defined(SSD1309_USE_SPI)
    nrf_drv_spi_config_t oled_spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    oled_spi_config.ss_pin   = OLED_SS_PIN;
    oled_spi_config.mosi_pin = OLED_MOSI_PIN;
    oled_spi_config.sck_pin  = OLED_SCK_PIN;
#endif

#if SPI_USE_INTERRUPT
    APP_ERROR_CHECK(nrf_drv_spi_init(&m_oled_spi, &spi_config, oled_spi_callback, NULL));
#else
    APP_ERROR_CHECK(nrf_drv_spi_init(&m_oled_spi, &oled_spi_config, NULL, NULL));
#endif

#if defined(SSD1309_USE_SPI)
    nrf_gpio_cfg_output(OLED_ENABLE_PIN);
    nrf_gpio_cfg_output(OLED_RES_PIN);
    nrf_gpio_cfg_output(OLED_DC_PIN);
#endif
}

void oled_init()
{
    oled_enable();
    ssd1309_Init(spi_oled_comm_handle);
    ssd1309_Fill(Black);
}

int main(void)
{
	/* Initialize log. */
    log_init();   
	
	spi_init();
	
    /* Initialize oled. */
    oled_init();	
	oled_write_symbol(BLE, 32, 0);
	oled_write_string("Testing 123!!", 0, 32);
	
    while (1)
    {
		NRF_LOG_FLUSH();
	}
}

