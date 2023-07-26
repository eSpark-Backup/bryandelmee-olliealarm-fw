#include "lcd_spi_driver.h"

static spi_device_handle_t lcd_spi_dev_handle = NULL; 

/**
 * @brief Startup LCD Driver :
 * Writes the initialization sequence of ST7701SN over SPI interface.
 * 
 * @return esp_err_t 
 */
esp_err_t lcd_driver_startup()
{
    esp_err_t ret = ESP_OK;
    ret |= lcd_spi_init();
    ret |= lcd_reg_init_sequence();
    ret |= lcd_spi_deinit();

    return ret;   
}

esp_err_t lcd_spi_init()
{
    esp_err_t ret = ESP_OK;
    ret |= lcd_spi_bus_init();
    ret |= lcd_spi_bus_add_device();

    return ret;
}

esp_err_t lcd_spi_deinit()
{
    esp_err_t ret = ESP_OK;
    ret |= spi_bus_remove_device(lcd_spi_dev_handle);
    ret |= spi_bus_free(LCD_SPI_HOST);

    return ret;
}

esp_err_t lcd_spi_bus_init()
{
    spi_bus_config_t buscfg = {
        .sclk_io_num = LCD_GPIO_SCK,
        .mosi_io_num = LCD_GPIO_SDA,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 10 * 1024,
    };

    return spi_bus_initialize(LCD_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
}

esp_err_t lcd_spi_bus_add_device()
{
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = SPI_MASTER_FREQ_10M,
        .mode = 0,                              
        .spics_io_num = LCD_GPIO_CS,            
        .queue_size = 7,                        
    };
    
    return spi_bus_add_device(LCD_SPI_HOST, &devcfg, &lcd_spi_dev_handle);
}

esp_err_t lcd_spi_write_cmd(const uint16_t cmd)
{
    uint16_t tmp_cmd =(cmd | 0x0000);
    spi_transaction_ext_t trans =(spi_transaction_ext_t) {
        .base = {
            .flags = SPI_TRANS_VARIABLE_CMD,
            .cmd = tmp_cmd,
        },
        .command_bits = 9,
    };
    return spi_device_transmit(lcd_spi_dev_handle, (spi_transaction_t *)&trans);
}

esp_err_t lcd_spi_write_data(const uint16_t data) 
{
    uint16_t tmp_data =(data | 0x0100);
    spi_transaction_ext_t trans =(spi_transaction_ext_t) 
    {
        .base = {
            .flags = SPI_TRANS_VARIABLE_CMD,
            .cmd = tmp_data,
        },
        .command_bits = 9,
    };
    return spi_device_transmit(lcd_spi_dev_handle, (spi_transaction_t *)&trans);
}

esp_err_t lcd_reg_init_sequence()
{
    esp_err_t ret = ESP_OK;
    ret |= lcd_spi_write_cmd(ST7701_SWRESET);
    ret |= lcd_spi_write_data(0x01);

    // Command2, BK0
    ret |= lcd_spi_write_cmd(ST7701_CND2BKxSEL); 
    ret |= lcd_spi_write_data(0x77);   
    ret |= lcd_spi_write_data(0x01);   
    ret |= lcd_spi_write_data(0x00);   
    ret |= lcd_spi_write_data(0x00);   
    ret |= lcd_spi_write_data(0x13);    
 
    ret |= lcd_spi_write_cmd(0xFF);     
    ret |= lcd_spi_write_data(0x77);   
    ret |= lcd_spi_write_data(0x01);   
    ret |= lcd_spi_write_data(0x00);   
    ret |= lcd_spi_write_data(0x00);   
    ret |= lcd_spi_write_data(0x10);   

    
    /* Display Line setting */
    ret |= lcd_spi_write_cmd(ST7701_BK0_LNESET);     
    ret |= lcd_spi_write_data(0x3B);    //Line[6:0] : display line setting, LDE_EN : add extra-line enable 
    ret |= lcd_spi_write_data(0x00);    //LDE_EN=”0”,no add delta line
    
    /* Porch control */
    ret |= lcd_spi_write_cmd(ST7701_BK0_PORCTRL);     
    ret |= lcd_spi_write_data(0x0F);    //VBP: Back-Porch Vertical line setting for display. 
    ret |= lcd_spi_write_data(0x3C);    //VFP: Front-Porch Vertical line setting for display.

    /* Inversion selection & Frame Rate Control */
    ret |= lcd_spi_write_cmd(ST7701_BK0_INVSET);      
    ret |= lcd_spi_write_data(0x07);
    ret |= lcd_spi_write_data(0x3C);    //Frame Rate : 60 (previous val : 0x0A)
 
    ret |= lcd_spi_write_cmd(0xC3);
    ret |= lcd_spi_write_data(0x02);
 
    ret |= lcd_spi_write_cmd(0xCC);     
    ret |= lcd_spi_write_data(0x10);  
    
    ret |= lcd_spi_write_cmd(0xB0);     
    ret |= lcd_spi_write_data(0x05);   
    ret |= lcd_spi_write_data(0x12);   
    ret |= lcd_spi_write_data(0x98);   
    ret |= lcd_spi_write_data(0x0E);   
    ret |= lcd_spi_write_data(0x0F);   
    ret |= lcd_spi_write_data(0x07);   
    ret |= lcd_spi_write_data(0x07);   
    ret |= lcd_spi_write_data(0x09);   
    ret |= lcd_spi_write_data(0x09);   
    ret |= lcd_spi_write_data(0x23);   
    ret |= lcd_spi_write_data(0x05);   
    ret |= lcd_spi_write_data(0x52);   
    ret |= lcd_spi_write_data(0x0F);   
    ret |= lcd_spi_write_data(0x67);   
    ret |= lcd_spi_write_data(0x2C);   
    ret |= lcd_spi_write_data(0x11);   
    
    ret |= lcd_spi_write_cmd(0xB1);     
    ret |= lcd_spi_write_data(0x0B);   
    ret |= lcd_spi_write_data(0x11);   
    ret |= lcd_spi_write_data(0x97);   
    ret |= lcd_spi_write_data(0x0C);   
    ret |= lcd_spi_write_data(0x12);   
    ret |= lcd_spi_write_data(0x06);   
    ret |= lcd_spi_write_data(0x06);   
    ret |= lcd_spi_write_data(0x08);   
    ret |= lcd_spi_write_data(0x08);   
    ret |= lcd_spi_write_data(0x22);   
    ret |= lcd_spi_write_data(0x03);   
    ret |= lcd_spi_write_data(0x51);   
    ret |= lcd_spi_write_data(0x11);   
    ret |= lcd_spi_write_data(0x66);   
    ret |= lcd_spi_write_data(0x2B);   
    ret |= lcd_spi_write_data(0x0F);   
    
    ret |= lcd_spi_write_cmd(0xFF);     
    ret |= lcd_spi_write_data(0x77);   
    ret |= lcd_spi_write_data(0x01);   
    ret |= lcd_spi_write_data(0x00);   
    ret |= lcd_spi_write_data(0x00);   
    ret |= lcd_spi_write_data(0x11);   
    
    ret |= lcd_spi_write_cmd(0xB0);     
    ret |= lcd_spi_write_data(0x5D); 
    
    ret |= lcd_spi_write_cmd(0xB1);      
    ret |= lcd_spi_write_data(0x2D);   
    
    ret |= lcd_spi_write_cmd(0xB2);     
    ret |= lcd_spi_write_data(0x81);   
    
    ret |= lcd_spi_write_cmd(0xB3);     
    ret |= lcd_spi_write_data(0x80);   
    
    ret |= lcd_spi_write_cmd(0xB5);     
    ret |= lcd_spi_write_data(0x4E);   
    
    ret |= lcd_spi_write_cmd(0xB7);     
    ret |= lcd_spi_write_data(0x85);   
    
    ret |= lcd_spi_write_cmd(0xB8);     
    ret |= lcd_spi_write_data(0x20);   
    
    ret |= lcd_spi_write_cmd(0xC1);     
    ret |= lcd_spi_write_data(0x78);   
    
    ret |= lcd_spi_write_cmd(0xC2);     
    ret |= lcd_spi_write_data(0x78);   
    
    ret |= lcd_spi_write_cmd(0xD0);     
    ret |= lcd_spi_write_data(0x88);   
    
    ret |= lcd_spi_write_cmd(0xE0);     
    ret |= lcd_spi_write_data(0x00);   
    ret |= lcd_spi_write_data(0x00);   
    ret |= lcd_spi_write_data(0x02);   
    
    ret |= lcd_spi_write_cmd(0xE1);     
    ret |= lcd_spi_write_data(0x06);   
    ret |= lcd_spi_write_data(0x30);   
    ret |= lcd_spi_write_data(0x08);   
    ret |= lcd_spi_write_data(0x30);   
    ret |= lcd_spi_write_data(0x05);   
    ret |= lcd_spi_write_data(0x30);   
    ret |= lcd_spi_write_data(0x07);   
    ret |= lcd_spi_write_data(0x30);   
    ret |= lcd_spi_write_data(0x00);   
    ret |= lcd_spi_write_data(0x33);   
    ret |= lcd_spi_write_data(0x33);   
    
    ret |= lcd_spi_write_cmd(0xE2);     
    ret |= lcd_spi_write_data(0x11);   
    ret |= lcd_spi_write_data(0x11);   
    ret |= lcd_spi_write_data(0x33);   
    ret |= lcd_spi_write_data(0x33);   
    ret |= lcd_spi_write_data(0xF4);   
    ret |= lcd_spi_write_data(0x00);   
    ret |= lcd_spi_write_data(0x00);   
    ret |= lcd_spi_write_data(0x00);   
    ret |= lcd_spi_write_data(0xF4);   
    ret |= lcd_spi_write_data(0x00);   
    ret |= lcd_spi_write_data(0x00);   
    ret |= lcd_spi_write_data(0x00);   
    
    ret |= lcd_spi_write_cmd(0xE3);     
    ret |= lcd_spi_write_data(0x00);   
    ret |= lcd_spi_write_data(0x00);   
    ret |= lcd_spi_write_data(0x11);   
    ret |= lcd_spi_write_data(0x11);   
    
    ret |= lcd_spi_write_cmd(0xE4);     
    ret |= lcd_spi_write_data(0x44);   
    ret |= lcd_spi_write_data(0x44);   
    
    ret |= lcd_spi_write_cmd(0xE5);     
    ret |= lcd_spi_write_data(0x0D);   
    ret |= lcd_spi_write_data(0xF5);   
    ret |= lcd_spi_write_data(0x30);   
    ret |= lcd_spi_write_data(0xF0);   
    ret |= lcd_spi_write_data(0x0F);   
    ret |= lcd_spi_write_data(0xF7);   
    ret |= lcd_spi_write_data(0x30);   
    ret |= lcd_spi_write_data(0xF0);   
    ret |= lcd_spi_write_data(0x09);   
    ret |= lcd_spi_write_data(0xF1);   
    ret |= lcd_spi_write_data(0x30);   
    ret |= lcd_spi_write_data(0xF0);   
    ret |= lcd_spi_write_data(0x0B);   
    ret |= lcd_spi_write_data(0xF3);   
    ret |= lcd_spi_write_data(0x30);   
    ret |= lcd_spi_write_data(0xF0);   
    
    ret |= lcd_spi_write_cmd(0xE6);     
    ret |= lcd_spi_write_data(0x00);   
    ret |= lcd_spi_write_data(0x00);   
    ret |= lcd_spi_write_data(0x11);   
    ret |= lcd_spi_write_data(0x11);   
    
    ret |= lcd_spi_write_cmd(0xE7);     
    ret |= lcd_spi_write_data(0x44);   
    ret |= lcd_spi_write_data(0x44);   
    
    ret |= lcd_spi_write_cmd(0xE8);     
    ret |= lcd_spi_write_data(0x0C);   
    ret |= lcd_spi_write_data(0xF4);   
    ret |= lcd_spi_write_data(0x30);   
    ret |= lcd_spi_write_data(0xF0);   
    ret |= lcd_spi_write_data(0x0E);   
    ret |= lcd_spi_write_data(0xF6);   
    ret |= lcd_spi_write_data(0x30);   
    ret |= lcd_spi_write_data(0xF0);   
    ret |= lcd_spi_write_data(0x08);   
    ret |= lcd_spi_write_data(0xF0);   
    ret |= lcd_spi_write_data(0x30);   
    ret |= lcd_spi_write_data(0xF0);   
    ret |= lcd_spi_write_data(0x0A);   
    ret |= lcd_spi_write_data(0xF2);   
    ret |= lcd_spi_write_data(0x30);   
    ret |= lcd_spi_write_data(0xF0);   
    
    ret |= lcd_spi_write_cmd(0xE9);     
    ret |= lcd_spi_write_data(0x36);   
    ret |= lcd_spi_write_data(0x01);   
    
    ret |= lcd_spi_write_data(0xEB);     
    ret |= lcd_spi_write_data(0x00);   
    ret |= lcd_spi_write_data(0x01);   
    ret |= lcd_spi_write_data(0xE4);   
    ret |= lcd_spi_write_data(0xE4);   
    ret |= lcd_spi_write_data(0x44);   
    ret |= lcd_spi_write_data(0x88);   
    ret |= lcd_spi_write_data(0x40);   

    ret |= lcd_spi_write_cmd(0xED);  //  FW    OK 
    ret |= lcd_spi_write_data(0xFF);   
    ret |= lcd_spi_write_data(0x10);   
    ret |= lcd_spi_write_data(0xAF);  //    BW BF 
    ret |= lcd_spi_write_data(0x76);   
    ret |= lcd_spi_write_data(0x54);   
    ret |= lcd_spi_write_data(0x2B);  //    BW2A
    ret |= lcd_spi_write_data(0xCF);   //    BW FC 
    ret |= lcd_spi_write_data(0xFF);   
    ret |= lcd_spi_write_data(0xFF);   
    ret |= lcd_spi_write_data(0xFC);   //    BW CF 
    ret |= lcd_spi_write_data(0xB2);   //    BW  A2 
    ret |= lcd_spi_write_data(0x45);   
    ret |= lcd_spi_write_data(0x67);   
    ret |= lcd_spi_write_data(0xFA);   //    BW  FB
    ret |= lcd_spi_write_data(0x01);   
    ret |= lcd_spi_write_data(0xFF);   
 
    ret |= lcd_spi_write_cmd(0xEF);     
    ret |= lcd_spi_write_data(0x08);   
    ret |= lcd_spi_write_data(0x08);   
    ret |= lcd_spi_write_data(0x08);   
    ret |= lcd_spi_write_data(0x45);   
    ret |= lcd_spi_write_data(0x3F);   
    ret |= lcd_spi_write_data(0x54);  

    ret |= lcd_spi_write_cmd(0xFF);     
    ret |= lcd_spi_write_data(0x77);   
    ret |= lcd_spi_write_data(0x01);   
    ret |= lcd_spi_write_data(0x00);   
    ret |= lcd_spi_write_data(0x00);   
    ret |= lcd_spi_write_data(0x00);    
    ret |= lcd_spi_write_cmd( 0x11);     

    vTaskDelay(pdMS_TO_TICKS(120));                //ms

    ret |= lcd_spi_write_cmd(0x3A);     
    ret |= lcd_spi_write_data(0x60);   
    ret |= lcd_spi_write_cmd(ST7701_MADCTL);     
    ret |= lcd_spi_write_data(0x00);
    ret |= lcd_spi_write_cmd(0x53);   // Brightness COntrol On  
    ret |= lcd_spi_write_data(0x28);
    ret |= lcd_spi_write_cmd(0x5E);   //  Write CABC Minimum Brightness
    ret |= lcd_spi_write_data(0x00);
    ret |= lcd_spi_write_cmd(0x35);     
    ret |= lcd_spi_write_data(0x00);   
    ret |= lcd_spi_write_cmd(0x29);   

    vTaskDelay(pdMS_TO_TICKS(20)); 
    return ret;
}




