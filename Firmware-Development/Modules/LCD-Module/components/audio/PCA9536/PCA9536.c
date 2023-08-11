#include "PCA9536.h"

/**
 * @brief i2c master initialization
 */
esp_err_t pca9536_i2c_master_init(void)
{  
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = PCA9536_SDA_PIN,
        .scl_io_num = PCA9536_SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = PCA9536_CLOCK_SPEED,
    };
    esp_err_t ret; 
    ret = i2c_param_config(PCA9636_I2C_MASTER_NUM, &conf);
    ret = i2c_driver_install(PCA9636_I2C_MASTER_NUM, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);

    return ret;
}

/**
 * @brief Deinitialized PCA9536 I2C driver & release allocated resources.
 */
esp_err_t pca9536_i2c_deinit(void)
{ 
    return i2c_driver_delete(PCA9636_I2C_MASTER_NUM);
}

/**
 * @brief Read a sequence of bytes from a PCA9536 IC registers
 */
static inline esp_err_t pca9536_register_read(uint8_t reg_addr, uint8_t *data, size_t len)
{
    return i2c_master_write_read_device(PCA9636_I2C_MASTER_NUM, PCA9536_DEVICE_ADDR, &reg_addr, 1, data, len, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

/**
 * @brief Write a byte to a PCA9536 IC register
 */
static esp_err_t pca9536_register_write_byte(uint8_t reg_addr, uint8_t data)
{
    esp_err_t ret;
    uint8_t write_buf[2] = {reg_addr, data};

    ret = i2c_master_write_to_device(PCA9636_I2C_MASTER_NUM, PCA9536_DEVICE_ADDR, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);

    return ret;
}

esp_err_t pca9536_set_pin_mode_output(uint8_t io_num)
{
    esp_err_t ret;
    uint8_t pca9536_data;
    pca9536_register_read(PCA9536_REG_CONFIG, &pca9536_data, 1);
    
    pca9536_data &= ~(1 << io_num);
    ret = pca9536_register_write_byte(PCA9536_REG_CONFIG, pca9536_data);
    return ret;
}

esp_err_t pca9536_set_pin_high(uint8_t io_num)
{
    esp_err_t ret;
    uint8_t pca9536_data;

    ret = pca9536_register_read(PCA9536_REG_OUTPUT, &pca9536_data, 1);
    pca9536_data |= (1 << io_num);
    ret = pca9536_register_write_byte(PCA9536_REG_OUTPUT, pca9536_data);

    return ret;
}
esp_err_t pca9536_set_pin_low(uint8_t io_num)
{
    esp_err_t ret;
    uint8_t pca9536_data;

    ret = pca9536_register_read(PCA9536_REG_OUTPUT, &pca9536_data, 1);
    pca9536_data &= ~(1 << io_num);
    ret = pca9536_register_write_byte(PCA9536_REG_OUTPUT, pca9536_data);

    return ret;
}


