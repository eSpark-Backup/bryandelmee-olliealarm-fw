#ifndef _ES8388_I2C_H__
#define _ES8388_I2C_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_check.h"
#include "driver/i2c.h"

/* ES8388 address */
#define ES8388_ADDR 0x10  /*!< 0x11:CE=1;0x10:CE=0*/

esp_err_t es_i2c_init(int sda_pin, int scl_pin);
esp_err_t es_write_reg(uint8_t slave_addr, uint8_t reg_add, uint8_t data);
esp_err_t es_read_reg(uint8_t reg_add, uint8_t *p_data);

#ifdef __cplusplus
}
#endif

#endif //_ES8388_I2C_H__