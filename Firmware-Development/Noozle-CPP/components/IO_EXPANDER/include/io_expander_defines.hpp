#pragma once

/* I2C uC Configurations */
#define PCA9636_I2C_MASTER_NUM      I2C_NUM_0                   /* PCA9536 I2C Port Number */
#define PCA9536_SDA_PIN             GPIO_NUM_40                 /* PCA9536 SDA PIN */
#define PCA9536_SCL_PIN             GPIO_NUM_41                 /* PCA9536 SDA PIN */
#define PCA9536_CLOCK_SPEED         100000                      /* PCA9536 I2C Speed 100Kbps */
#define I2C_MASTER_TIMEOUT_MS       1000
#define I2C_MASTER_TX_BUF_DISABLE   0                           /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                           /*!< I2C master doesn't need buffer */

/* Register Addresses */
#define PCA9536_DEVICE_ADDR         0x41        /*!< Slave address of the PCA9536 sensor */
#define PCA9536_REG_INPUT           0x00        // Input Port Register           (R)    B00000000 (Default)
#define PCA9536_REG_OUTPUT          0x01        // Output Port Register          (R/W)  B00000001
#define PCA9536_REG_POLARITY        0x02        // Polarity Inversion Register   (R/W)  B00000010
#define PCA9536_REG_CONFIG          0x03        // Configuration Register        (R/W)  B00000011

/* GPIOS */
#define PCA9536_IO0                 0
#define PCA9536_IO1                 1
#define PCA9536_IO2                 2
#define PCA9536_IO3                 3