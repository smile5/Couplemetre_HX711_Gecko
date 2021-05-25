/*
 * hx711.c
 *
 *  Created on: 4 févr. 2021
 *      Author: xgaltier
 */

#include "hx711.h"

__STATIC_INLINE void
delayUs2 (uint32_t us)
{
  uint32_t clockCycles = (CMU_ClockFreqGet (cmuClock_CORE) / 1000000) * us; // divide clock (big number) by 1E6 instead of us (often a small number)

#define CALC_CYCLES 121 // 105 cycles from function call to DWT->CYCCNT = 0; and 16 from if(primask) to exiting
  if (clockCycles > CALC_CYCLES)
    {
      clockCycles -= CALC_CYCLES; // correct for time required to call function and calculate cycles
    }
  else
    {
      clockCycles = 0; // can't fully correct, calculation takes longer than requested delay -> make delay as short as possible, about 11us with 11MHz clock
    }

  DWT->CTRL |= 0x01; // enable ARM cycle count

  DWT->CYCCNT = 0; // set cycle count to 0
  while (DWT->CYCCNT < clockCycles)
    {
    }
}

void HX711_config()
{
  GPIO_PinModeSet(DOUT,gpioModeInput,0);
  GPIO_PinModeSet(PD_SCK,gpioModePushPull ,0);

}

bool digitalRead(GPIO_Port_TypeDef port, unsigned char pin)
{
  return GPIO_PinInGet(port,pin);
}

void digitalWrite(GPIO_Port_TypeDef port, unsigned char pin, bool value)
{
  if (value== true)
    {
      GPIO_PinOutSet(port,pin);
    }
  else
    {
      GPIO_PinOutClear(port,pin);
    }
}

bool HX711_Is_Ready() {
  return digitalRead(DOUT) == false;
}

long HX711_Read(unsigned char gain) {
  byte data[3];
  char i,j;

  // wait for the chip to become ready
  while (!HX711_Is_Ready());
  switch (gain) {
case 128:   // channel A, gain factor 128
  gain = 1;
  break;
case 64:    // channel A, gain factor 64
  gain = 3;
  break;
case 32:    // channel B, gain factor 32
  gain = 2;
  break;
}
  //sl_udelay_wait (2);
  delayUs2(2);
  // pulse the clock pin 24 times to read the data
  for (j = 3; j--;) {
    for ( i = 8; i--;) {
      digitalWrite(PD_SCK, true);
      //sl_udelay_wait (2);
      delayUs2(2);
      bitWrite(data[j], i, digitalRead(DOUT));
      digitalWrite(PD_SCK, false);
      //sl_udelay_wait (2);
      delayUs2(2);
    }
  }

  // set the channel and the gain factor for the next reading using the clock pin
  for (int i = 0; i < gain; i++) {
    digitalWrite(PD_SCK, true);
    //sl_udelay_wait (2);
    delayUs2(2);
    digitalWrite(PD_SCK, false);
    //sl_udelay_wait (2);
    delayUs2(2);
  }

  data[2] ^= 0x80;

  return ((uint32_t) data[2] << 16) | ((uint32_t) data[1] << 8) | (uint32_t) data[0];
}


void HX711_power_down() {
  digitalWrite(PD_SCK, false);
  digitalWrite(PD_SCK, true);
}

void HX711_power_up() {
  digitalWrite(PD_SCK, false);
}

