#include "io_expander.hpp"
#include "io_expander_defines.hpp"
#include "esp_check.h"
#include "driver/gpio.h"
#include <driver/i2c.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static const char* TAG = "IO_EXPANDER";

io_expander::io_expander()
{
    initI2C();
}

io_expander::~io_expander()
{
    // deinitI2C();
}


/**
 * @brief i2c master initialization
 */
esp_err_t io_expander::initI2C(void)
{  
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = GPIO_NUM_40;
    conf.scl_io_num = GPIO_NUM_41;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = (uint32_t) 100000;
    conf.clk_flags = 0;
 
    ESP_RETURN_ON_ERROR(i2c_param_config(PCA9636_I2C_MASTER_NUM, &conf), TAG, "initI2C: Setting up I2C bus configuration");
    ESP_RETURN_ON_ERROR(i2c_driver_install(PCA9636_I2C_MASTER_NUM, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0), TAG, "initI2C : Install I2C driver");
    return ESP_OK;
}

/**
 * @brief Deinitialized PCA9536 I2C driver & release allocated resources.
 */
esp_err_t io_expander::deinitI2C(void)
{ 
    ESP_RETURN_ON_ERROR(i2c_driver_delete(PCA9636_I2C_MASTER_NUM), TAG, "deinitI2C : Deletes I2C driver");
    return ESP_OK;
}

/**
 * @brief Read a sequence of bytes from a PCA9536 IC registers
 */
esp_err_t io_expander::i2cRead(uint8_t reg_addr, uint8_t *data, size_t len)
{
    ESP_RETURN_ON_ERROR(i2c_master_write_read_device(PCA9636_I2C_MASTER_NUM, PCA9536_DEVICE_ADDR, &reg_addr, 1, data, len, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS), TAG, "i2cRead: Reading bytes on I2C  ");
    return ESP_OK;
}

/**
 * @brief Write a byte to a PCA9536 IC register
 */
esp_err_t io_expander::i2cWrite(uint8_t reg_addr, uint8_t data)
{
    uint8_t write_buf[2] = {reg_addr, data};
    ESP_RETURN_ON_ERROR(i2c_master_write_to_device(PCA9636_I2C_MASTER_NUM, PCA9536_DEVICE_ADDR, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS), TAG, "i2cWrite: Writing Bytes on I2C");
    return ESP_OK;
}

esp_err_t io_expander::setOutputPin(uint8_t io_num)
{
    uint8_t pca9536_data;
    i2cRead(PCA9536_REG_CONFIG, &pca9536_data, 1);
    pca9536_data &= ~(1 << io_num);
    ESP_RETURN_ON_ERROR(i2cWrite(PCA9536_REG_CONFIG, pca9536_data), TAG, "setOutputPin: Setting pin-mode as output");
    return ESP_OK;
}

esp_err_t io_expander::setPinHigh(uint8_t io_num)
{
    uint8_t pca9536_data;
    ESP_RETURN_ON_ERROR(i2cRead(PCA9536_REG_OUTPUT, &pca9536_data, 1), TAG, "setPinHigh: Reading output register to get all pins states");
    pca9536_data |= (1 << io_num);
    ESP_RETURN_ON_ERROR(i2cWrite(PCA9536_REG_OUTPUT, pca9536_data), TAG, "setPinHigh: Writing High on the desired pin");
    return ESP_OK;
}

esp_err_t io_expander::setPinLow(uint8_t io_num)
{
    uint8_t pca9536_data;
    ESP_RETURN_ON_ERROR(i2cRead(PCA9536_REG_OUTPUT, &pca9536_data, 1), TAG, "setPinLow:  Reading output register to get all pins states");
    pca9536_data &= ~(1 << io_num);
    ESP_RETURN_ON_ERROR(i2cWrite(PCA9536_REG_OUTPUT, pca9536_data), TAG, "setPinLow: Writing LOW on the desired pin");
    return ESP_OK;
}


