#ifndef _LCD_DRIVER_REG_H_
#define _LCD_DRIVER_REG_H_

#define ST7701_SWRESET      0x01    ///< Software Reset
#define ST7701_CND2BKxSEL   0xFF    ///< Command2 BKx Selection
#define ST7701_BK0_PORCTRL  0xC1    ///< Porch control
#define ST7701_BK0_LNESET   0xC0    ///< Display Line setting
#define ST7701_BK0_INVSET   0xC2    ///< Inversion selection & Frame Rate Control
#define ST7701_MADCTL       0x36    ///< Display data access control

#endif