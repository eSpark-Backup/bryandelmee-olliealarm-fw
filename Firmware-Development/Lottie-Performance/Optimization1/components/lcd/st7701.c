/*
 * SPDX-FileCopyrightText: 2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "st7701.h"

static const char *TAG = "st7701";

static spi_device_handle_t rgb_screen_spi = NULL;

#define DISPLAY_CMD(cmd, params...)                                \
    {                                                              \
        const uint8_t d[] = {params};                              \
        sendCommandToDisplay(cmd, sizeof(d) / sizeof(uint8_t), d); \
    }  ///< Macro to automatically call sendCommandToDisplay

static void spi_soft_init(void)
{
    spi_bus_config_t buscfg = {
        .sclk_io_num = GPIO_LCD_SCK,
        .mosi_io_num = GPIO_LCD_SDA,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 10 * 1024,
    };
    spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = SPI_MASTER_FREQ_10M,   //Clock out at 20 MHz
        .mode = 0,                               //SPI mode 0
        .spics_io_num = GPIO_LCD_CS,              //CS pin
        .queue_size = 7,                         //We want to be able to queue 7 transactions at a time
    };
    
    spi_bus_add_device(SPI2_HOST, &devcfg, &rgb_screen_spi);

	// spi_bus_remove_device(rgb_screen_spi);
	// spi_bus_free(SPI2_HOST);
}
static void lcd_cmd(spi_device_handle_t spi, const uint16_t data) {
    esp_err_t ret;
    uint16_t tmp_cmd =(data | 0x0000);
    spi_transaction_ext_t trans =(spi_transaction_ext_t) {
        .base = {
            .flags = SPI_TRANS_VARIABLE_CMD,
            .cmd = tmp_cmd,
        },
        .command_bits = 9,
    };
    ret = spi_device_transmit(spi,(spi_transaction_t *)&trans);
    assert(ret == ESP_OK);          //Should have had no issues.
}

static void lcd_data(spi_device_handle_t spi, const uint16_t data) {
    esp_err_t ret;
    uint16_t tmp_cmd =(data | 0x0100);
    spi_transaction_ext_t trans =(spi_transaction_ext_t) {
        .base = {
            .flags = SPI_TRANS_VARIABLE_CMD,
            .cmd = tmp_cmd,
        },
        .command_bits = 9,
    };
    ret = spi_device_transmit(spi,(spi_transaction_t *)&trans);

    assert(ret == ESP_OK);          //Should have had no issues.
}

static void st7701_write_cmd(uint16_t cmd) {
    lcd_cmd(rgb_screen_spi, cmd);
}

static void st7701_write_data(uint16_t data) {
    lcd_data(rgb_screen_spi, data);
}

void st7701_reg_init(void)
{
    spi_soft_init();   //SPI - init
    ESP_LOGI(TAG, "st7701 register init");

    // Command2, BK0
    st7701_write_cmd(ST7701_CND2BKxSEL); 
    st7701_write_data(0x77);   
    st7701_write_data(0x01);   
    st7701_write_data(0x00);   
    st7701_write_data(0x00);   
    st7701_write_data(0x13);   
    
    /* EFh not found in datasheet*/
    // st7701_write_cmd(0xEF);     
    // st7701_write_data(0x08);   
 
    st7701_write_cmd(0xFF);     
    st7701_write_data(0x77);   
    st7701_write_data(0x01);   
    st7701_write_data(0x00);   
    st7701_write_data(0x00);   
    st7701_write_data(0x10);   

    
    /* Display Line setting */
    st7701_write_cmd(ST7701_BK0_LNESET);     
    st7701_write_data(0x3B);    //Line[6:0] : display line setting, LDE_EN : add extra-line enable 
    st7701_write_data(0x00);    //LDE_EN=”0”,no add delta line
    
    /* Porch control */
    st7701_write_cmd(ST7701_BK0_PORCTRL);     
    st7701_write_data(0x0F);    //VBP: Back-Porch Vertical line setting for display. 
    st7701_write_data(0x3C);    //VFP: Front-Porch Vertical line setting for display.

    /* Inversion selection & Frame Rate Control */
    st7701_write_cmd(ST7701_BK0_INVSET);      
    st7701_write_data(0x07);
    st7701_write_data(0x3C);    //Frame Rate : 60 (previous val : 0x0A)

 
    st7701_write_cmd(0xC3);
    st7701_write_data(0x02);// 00
    // st7701_write_data(0x82);
    // st7701_write_data(0x82);

//     st7701_write_cmd(0xCD);
//     st7701_write_data(0x04);

// st7701_write_cmd(0xC7);     
// st7701_write_data(0x04);  //    04 FW   00  bw  
// 
st7701_write_cmd(0xCC);     
st7701_write_data(0x10);  
//    
// 
st7701_write_cmd(0xB0);     
st7701_write_data(0x05);   
st7701_write_data(0x12);   
st7701_write_data(0x98);   
st7701_write_data(0x0E);   
st7701_write_data(0x0F);   
st7701_write_data(0x07);   
st7701_write_data(0x07);   
st7701_write_data(0x09);   
st7701_write_data(0x09);   
st7701_write_data(0x23);   
st7701_write_data(0x05);   
st7701_write_data(0x52);   
st7701_write_data(0x0F);   
st7701_write_data(0x67);   
st7701_write_data(0x2C);   
st7701_write_data(0x11);   
// 
st7701_write_cmd(0xB1);     
st7701_write_data(0x0B);   
st7701_write_data(0x11);   
st7701_write_data(0x97);   
st7701_write_data(0x0C);   
st7701_write_data(0x12);   
st7701_write_data(0x06);   
st7701_write_data(0x06);   
st7701_write_data(0x08);   
st7701_write_data(0x08);   
st7701_write_data(0x22);   
st7701_write_data(0x03);   
st7701_write_data(0x51);   
st7701_write_data(0x11);   
st7701_write_data(0x66);   
st7701_write_data(0x2B);   
st7701_write_data(0x0F);   
// 
st7701_write_cmd(0xFF);     
st7701_write_data(0x77);   
st7701_write_data(0x01);   
st7701_write_data(0x00);   
st7701_write_data(0x00);   
st7701_write_data(0x11);   
// 
st7701_write_cmd(0xB0);     
st7701_write_data(0x5D); 
// 
st7701_write_cmd(0xB1);      
st7701_write_data(0x2D);   // 20 Fw   Bw  35
// 
st7701_write_cmd(0xB2);     
st7701_write_data(0x81);   
// 
st7701_write_cmd(0xB3);     
st7701_write_data(0x80);   
// 
st7701_write_cmd(0xB5);     
st7701_write_data(0x4E);   
// 
st7701_write_cmd(0xB7);     
st7701_write_data(0x85);   
// 
st7701_write_cmd(0xB8);     
st7701_write_data(0x20);   
// 
st7701_write_cmd(0xC1);     
st7701_write_data(0x78);   
// 
st7701_write_cmd(0xC2);     
st7701_write_data(0x78);   
// 
st7701_write_cmd(0xD0);     
st7701_write_data(0x88);   
// 
st7701_write_cmd(0xE0);     
st7701_write_data(0x00);   
st7701_write_data(0x00);   
st7701_write_data(0x02);   
// 
st7701_write_cmd(0xE1);     
st7701_write_data(0x06);   
st7701_write_data(0x30);   
st7701_write_data(0x08);   
st7701_write_data(0x30);   
st7701_write_data(0x05);   
st7701_write_data(0x30);   
st7701_write_data(0x07);   
st7701_write_data(0x30);   
st7701_write_data(0x00);   
st7701_write_data(0x33);   
st7701_write_data(0x33);   
// 
st7701_write_cmd(0xE2);     
st7701_write_data(0x11);   
st7701_write_data(0x11);   
st7701_write_data(0x33);   
st7701_write_data(0x33);   
st7701_write_data(0xF4);   
st7701_write_data(0x00);   
st7701_write_data(0x00);   
st7701_write_data(0x00);   
st7701_write_data(0xF4);   
st7701_write_data(0x00);   
st7701_write_data(0x00);   
st7701_write_data(0x00);   
// 
st7701_write_cmd(0xE3);     
st7701_write_data(0x00);   
st7701_write_data(0x00);   
st7701_write_data(0x11);   
st7701_write_data(0x11);   
// 
st7701_write_cmd(0xE4);     
st7701_write_data(0x44);   
st7701_write_data(0x44);   
// 
st7701_write_cmd(0xE5);     
st7701_write_data(0x0D);   
st7701_write_data(0xF5);   
st7701_write_data(0x30);   
st7701_write_data(0xF0);   
st7701_write_data(0x0F);   
st7701_write_data(0xF7);   
st7701_write_data(0x30);   
st7701_write_data(0xF0);   
st7701_write_data(0x09);   
st7701_write_data(0xF1);   
st7701_write_data(0x30);   
st7701_write_data(0xF0);   
st7701_write_data(0x0B);   
st7701_write_data(0xF3);   
st7701_write_data(0x30);   
st7701_write_data(0xF0);   
// 
st7701_write_cmd(0xE6);     
st7701_write_data(0x00);   
st7701_write_data(0x00);   
st7701_write_data(0x11);   
st7701_write_data(0x11);   
// 
st7701_write_cmd(0xE7);     
st7701_write_data(0x44);   
st7701_write_data(0x44);   
// 
st7701_write_cmd(0xE8);     
st7701_write_data(0x0C);   
st7701_write_data(0xF4);   
st7701_write_data(0x30);   
st7701_write_data(0xF0);   
st7701_write_data(0x0E);   
st7701_write_data(0xF6);   
st7701_write_data(0x30);   
st7701_write_data(0xF0);   
st7701_write_data(0x08);   
st7701_write_data(0xF0);   
st7701_write_data(0x30);   
st7701_write_data(0xF0);   
st7701_write_data(0x0A);   
st7701_write_data(0xF2);   
st7701_write_data(0x30);   
st7701_write_data(0xF0);   
// 
st7701_write_cmd(0xE9);     
st7701_write_data(0x36);   
st7701_write_data(0x01);   
// 
st7701_write_data(0xEB);     
st7701_write_data(0x00);   
st7701_write_data(0x01);   
st7701_write_data(0xE4);   
st7701_write_data(0xE4);   
st7701_write_data(0x44);   
st7701_write_data(0x88);   
st7701_write_data(0x40);   
// 
st7701_write_cmd(0xED);  //  FW    OK 
st7701_write_data(0xFF);   
st7701_write_data(0x10);   
st7701_write_data(0xAF);  //    BW BF 
st7701_write_data(0x76);   
st7701_write_data(0x54);   
st7701_write_data(0x2B);  //    BW2A
st7701_write_data(0xCF);   //    BW FC 
st7701_write_data(0xFF);   
st7701_write_data(0xFF);   
st7701_write_data(0xFC);   //    BW CF 
st7701_write_data(0xB2);   //    BW  A2 
st7701_write_data(0x45);   
st7701_write_data(0x67);   
st7701_write_data(0xFA);   //    BW  FB
st7701_write_data(0x01);   
st7701_write_data(0xFF);   
// 
// 
//st7701_write_cmd(0xED);  //  BW    NG
//st7701_write_data(0xFF);   
//st7701_write_data(0x10);   
//st7701_write_data(0xBF);  //    BW BF 
//st7701_write_data(0x76);   
//st7701_write_data(0x54);   
//st7701_write_data(0x2A);  //    BW2A
//st7701_write_data(0xFC);   //    BW FC 
//st7701_write_data(0xFF);   
//st7701_write_data(0xFF);   
//st7701_write_data(0xCF);   //    BW CF 
//st7701_write_data(0xA2);   //    BW  A2 
//st7701_write_data(0x45);   
//st7701_write_data(0x67);   
//st7701_write_data(0xFB);   //    BW  FB
//st7701_write_data(0x01);   
//st7701_write_data(0xFF);   
// 
st7701_write_cmd(0xEF);     
st7701_write_data(0x08);   
st7701_write_data(0x08);   
st7701_write_data(0x08);   
st7701_write_data(0x45);   
st7701_write_data(0x3F);   
st7701_write_data(0x54);  

// 
st7701_write_cmd(0xFF);     
st7701_write_data(0x77);   
st7701_write_data(0x01);   
st7701_write_data(0x00);   
st7701_write_data(0x00);   
st7701_write_data(0x00);   
// 
st7701_write_cmd( 0x11);     
// 
vTaskDelay(pdMS_TO_TICKS(120));                //ms
st7701_write_cmd(0x3A);     
st7701_write_data(0x60);   
// 
st7701_write_cmd(ST7701_MADCTL);     
st7701_write_data(0x00);

st7701_write_cmd(0x53);   // Brightness COntrol On  
st7701_write_data(0x28);
st7701_write_cmd(0x5E);   //  Write CABC Minimum Brightness
st7701_write_data(0x00);

// 
st7701_write_cmd(0x35);     
st7701_write_data(0x00);   
// 
st7701_write_cmd(0x29);   
// 
vTaskDelay(pdMS_TO_TICKS(20)); 
}
void st7701_set_brightness_level(uint8_t brightness)
{
    st7701_write_cmd(0x51);     
    st7701_write_data(brightness);
}




