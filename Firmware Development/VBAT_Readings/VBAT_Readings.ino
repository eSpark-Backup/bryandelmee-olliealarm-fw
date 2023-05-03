#include <Adafruit_ADS1X15.h>
#include <SparkFun_PCA9536_Arduino_Library.h>
#include "USB.h"
#define Serial USBSerial

//Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */
Adafruit_ADS1015 ads;     /* Use this for the 12-bit version */

PCA9536 io;
#define VMOD_EN 0     /* Power-Amplifier Enable Pin on GPIO Expander PCA9635 */
void setupGPIOExpander(void);

void setup(void)
{
  Serial.begin(115200);
  Serial.println("Hello!");

  Serial.println("Getting single-ended readings from AIN0..3");
  Serial.println("ADC Range: +/- 6.144V (1 bit = 3mV/ADS1015, 0.1875mV/ADS1115)");

  setupGPIOExpander();

  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS.");
    while (1);
  }
}

void loop(void)
{
  int16_t adc0;
  float volts0;

  adc0 = ads.readADC_SingleEnded(0);
  volts0 = ads.computeVolts(adc0);

  Serial.println("-----------------------------------------------------------");
  Serial.print("AIN0: "); Serial.print(adc0); Serial.print("  "); Serial.print(volts0); Serial.println("V");

  delay(1000);
}

void setupGPIOExpander(void)
{
  Wire.begin(40,41);
  
  // Initialize the PCA9536 with a begin function
  io.begin();

  // pinMode can be used to set an I/O as OUTPUT or INPUT
  io.pinMode(VMOD_EN, OUTPUT);

  // Set the gpio-pins (HIGH)
  io.digitalWrite(VMOD_EN, HIGH); 
    
  Wire.end();
  
}
