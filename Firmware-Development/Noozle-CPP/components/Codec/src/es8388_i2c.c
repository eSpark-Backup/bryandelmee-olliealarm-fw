#include "es8388_i2c.h"
#include "es8388_reg.h"
#include "driver/i2c.h"

static const char* TAG = "es8388_i2c";

esp_err_t es_i2c_init(int sda_pin, int scl_pin)
{
    i2c_config_t es_i2c_cfg = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = sda_pin,
        .scl_io_num = scl_pin,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000
    };
    ESP_RETURN_ON_ERROR(i2c_param_config(I2C_NUM_0, (const i2c_config_t*) &es_i2c_cfg ), TAG, "i2c_param_config");  
    ESP_RETURN_ON_ERROR(i2c_driver_install(I2C_NUM_0, es_i2c_cfg.mode, 0, 0, 0), TAG, "i2c_driver_install");  
    
    return ESP_OK;
}

esp_err_t es_write_reg(uint8_t slave_addr, uint8_t reg_add, uint8_t data)
{
    uint8_t write_buf[2] = {reg_add, data};
    ESP_RETURN_ON_ERROR(i2c_master_write_to_device(I2C_NUM_0, slave_addr, write_buf, sizeof(write_buf), 1000 / portTICK_PERIOD_MS), TAG, "i2c_master_write_to_device");
    return ESP_OK;
}

esp_err_t es_read_reg(uint8_t reg_add, uint8_t *p_data)
{
    uint8_t read_buf[2] = {reg_add, p_data};
    ESP_RETURN_ON_ERROR(i2c_master_read_from_device(I2C_NUM_0, ES8388_ADDR, read_buf, sizeof(read_buf), 1000 / portTICK_PERIOD_MS), TAG, "i2c_master_read_from_device");
    return ESP_OK;
}