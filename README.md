# Attiny-i2c-adc-8bit

an Attiny85 2 channel 8 bit adc using TinywireS library

This code turns an Attiny85 into a 2-channel 8-bit adc commuicating via the i2c bus. 

 ![Sample Circuit](https://github.com/donquixote2u/Attiny-i2c-adc-8bit/blob/master/Attiny85-i2c-adc.jpg)

It has been tested with an ESP 8266 running Nodemcu lua firmware, but should be good for both 5v and 3v3 applications e.g. Arduino, Raspberry Pi.

The adc uses Attiny85 adc2 and adc3 (chip pins 2 and 3 respectively).

The data is loaded into i2c registers  0x02 and 0x03. 8-bit resolution means that the data for each adc only uses one i2c register
each.

Sampling the voltages on the adc pins is triggered by writing a byte containing the hex # of the adc required into i2c register  0x00.
The user should then delay reading registers 2 or 3 for the result by 200ms, to allow the sampling to finish; it uses the last 15 of 50 samples to aid accuracy.

For diagnostic purposes, I2C registers ox01 contains the # of the last ADC sampled; registers 0x04 and 0x05 contain the last (50th) adc sample value, and the adc config byte.

A sample lua module is provided that reads both channels. It is rather simple, and should be easily coverted to e.g. Arduino or Rasp Pi code.
