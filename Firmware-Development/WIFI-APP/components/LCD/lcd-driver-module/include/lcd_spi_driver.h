#ifndef _LCD_SPI_DRIVER_H_
#define _LCD_SPI_DRIVER_H_

#include <stdlib.h>
#include "esp_log.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"

#include "lcd_driver_pinouts.h"
#include "lcd_driver_reg.h"

esp_err_t lcd_driver_startup();
esp_err_t lcd_spi_init();
esp_err_t lcd_spi_deinit();
esp_err_t lcd_spi_bus_init();
esp_err_t lcd_spi_bus_add_device();
esp_err_t lcd_reg_init_sequence();

esp_err_t lcd_spi_write_cmd(const uint16_t cmd);
esp_err_t lcd_spi_write_data(const uint16_t data);

#endif

