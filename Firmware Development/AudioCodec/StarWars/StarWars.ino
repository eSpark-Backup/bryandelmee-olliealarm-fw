#include "AudioTools.h"
#include "StarWars30.h"
#include "es8388.h"
#include <SparkFun_PCA9536_Arduino_Library.h>
#include "USB.h"
#define Serial USBSerial

uint8_t channels = 1;
uint16_t sample_rate = 22050;
uint16_t bits_per_sample = 16;

MemoryStream music(StarWars30_raw, StarWars30_raw_len);
I2SStream i2s;  // Output to I2S
StreamCopy copier(i2s, music); // copies sound into i2s
PCA9536 io;
void setupPE();

void setup(){
    Serial.begin(115200);
    setupPE();
    auto config = i2s.defaultConfig(TX_MODE);
    config.sample_rate = sample_rate;
    config.channels = channels;
    config.bits_per_sample = 16;
    config.pin_ws = 45;
    config.pin_bck = 46;
    config.pin_data = 43;
    config.pin_data_rx = 44;
    config.pin_mck = 42;
    i2s.begin(config);
}

void loop(){
    copier.copy();
}

void setupPE()
{
  // Input/Output Modes
  es_dac_output_t output = (es_dac_output_t) ( DAC_OUTPUT_LOUT1 | DAC_OUTPUT_LOUT2 | DAC_OUTPUT_ROUT1 | DAC_OUTPUT_ROUT2 );
  es_adc_input_t input = ADC_INPUT_LINPUT2_RINPUT2;
  //  es_adc_input_t input = ADC_INPUT_LINPUT1_RINPUT1;
  
  Wire.begin(40,41);

  // Initialize the PCA9536 with a begin function
  if (io.begin() == false)
  {
    Serial.println("PCA9536 not detected. Please check wiring. Freezing...");
    while (1);
  }

  for (int i = 0; i < 4; i++)
  {
    // pinMode can be used to set an I/O as OUTPUT or INPUT
    io.pinMode(i, OUTPUT);
  }
  for (int i = 3; i >= 0; i--)
  {
    io.digitalWrite(i, HIGH); // Turn last pin HIGH
  }
  Wire.end();

  TwoWire wire(0);
  wire.setPins( 40, 41 );

  es8388 codec;
  codec.begin( &wire );
  codec.config( bits_per_sample, output, input, 90 );
}
