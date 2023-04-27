/*
 * SPDX-FileCopyrightText: 2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "st7701.h"

static const char *TAG = "st7701";

#define DISPLAY_CMD(cmd, params...)                                \
    {                                                              \
        const uint8_t d[] = {params};                              \
        sendCommandToDisplay(cmd, sizeof(d) / sizeof(uint8_t), d); \
    }  ///< Macro to automatically call sendCommandToDisplay


#if  0
    spi_device_handle_t spi;
    spi_bus_config_t buscfg={
        .miso_io_num = GPIO_NUM_NC,
        .mosi_io_num = GPIO_LCD_SDA,
        .sclk_io_num = GPIO_LCD_SCK,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz = 1024,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));

    spi_device_interface_config_t devcfg={
        .spics_io_num = GPIO_LCD_CS,               //CS pin        
        .clock_speed_hz = 10*1000*1000,           //Clock out at 10 MHz
        .mode = 0,                                //SPI mode 0
        .command_bits = 8,           //< Default amount of bits in command phase (0-16), used when ``SPI_TRANS_VARIABLE_CMD`` is not used, otherwise ignored.
        .address_bits = 8,  
        .queue_size = 10,            //We want to be able to queue 7 transactions at a time
    };
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &spi));

    // esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle);
    // esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle);
#endif
static void spi_soft_init(void)
{
#if 0
    gpio_pad_select_gpio(GPIO_LCD_CS);
	gpio_set_direction(GPIO_LCD_CS, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_LCD_CS, 1);   

	gpio_pad_select_gpio(GPIO_LCD_SCK);
	gpio_set_direction(GPIO_LCD_SCK, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_LCD_SCK, 1);  

	gpio_pad_select_gpio(GPIO_LCD_SDA);
	gpio_set_direction(GPIO_LCD_SDA, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_LCD_SDA, 1);  
#else
    gpio_config_t io_conf1 = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << GPIO_LCD_CS, 
    };
    gpio_config(&io_conf1);
    gpio_set_level(GPIO_LCD_CS, 1);

    gpio_config_t io_conf2 = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << GPIO_LCD_SCK, 
    };
    gpio_config(&io_conf2);
    gpio_set_level(GPIO_LCD_SCK, 1);

    gpio_config_t io_conf3 = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << GPIO_LCD_SDA,
    };
    gpio_config(&io_conf3);
    gpio_set_level(GPIO_LCD_SDA, 1);
#endif
}

static void spi_soft_write_9bits(uint16_t data)
{
	uint8_t i;
	LCD_CS_Clr();
	for(i = 0; i < 9; i++)
	{
        LCD_SCK_Clr();
		if(data & 0x100)   LCD_SDA_Set();
		else               LCD_SDA_Clr();
        LCD_SCK_Set();
		data <<= 1;
	}
	LCD_CS_Set();;
}

static void st7701_write_cmd(uint8_t cmd)
{
	uint16_t temp = 0;
	temp = temp | cmd;
	spi_soft_write_9bits(temp);
}

static void st7701_write_data(uint8_t data)
{
	uint16_t temp = 0x100;
	temp = temp | data;
	spi_soft_write_9bits(temp);
}

void st7701_reg_init(void)
{
    spi_soft_init();   //GPIO init
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
    st7701_write_data(0x10);    //VBP: Back-Porch Vertical line setting for display. 
    st7701_write_data(0x0c);    //VFP: Front-Porch Vertical line setting for display.

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
// 
// 
//??????**************************************************
    // 
//   st7701_write_cmd(0xFF);
//   st7701_write_data(0x77);
//    st7701_write_data(0x01);
//   st7701_write_data(0x00);
//  st7701_write_data(0x00);
//    st7701_write_data(0x12); 
// 
//   
//    st7701_write_cmd(0xD1);
//   st7701_write_data(0x81);
//  
//  
//   st7701_write_cmd(0xD2);
//   st7701_write_data(0x08);//08?data bus 06???? 
    //??????**********************************************
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
// 
st7701_write_cmd(0x35);     
st7701_write_data(0x00);   
// 
st7701_write_cmd(0x29);     
// 
vTaskDelay(pdMS_TO_TICKS(20)); 
}
// vTaskDelay(pdMS_TO_TICKS(50));




