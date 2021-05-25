#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_burtc.h"
#include "em_rmu.h"
#include "em_gpio.h"
#include "segmentlcd.h"
#include "udelay.h"

#include "hx711.h"

#define GAIN 64

#define COEFF_COUPLE 22.5677812e-6

// System ms tick counter
static uint32_t volatile msTicks = 0;

/**************************************************************************//**
 * @brief
 *    Triggered every ms
 *****************************************************************************/
void
SysTick_Handler (void)
{
  msTicks += 1; // Increment counter used in delay()
}

/**************************************************************************//**
 * @brief
 *    Busy wait for the specified number of milliseconds
 *
 * @param [in] msDelay
 *    Number of milliseconds to delay
 *****************************************************************************/
void
delay (uint32_t msDelay)
{
  uint32_t curTicks = msTicks;
  while ((msTicks - curTicks) < msDelay)
    {
    }
}

/*
 * @brief Delay for a given time in microseconds. Calculates required wait cycles depending on core clock and uses ARM's DWT cycle count (CYCCNT) to block until the cycles are expired.
 * Processing of the function itself takes about 121 clock cycles (11us at 11MHz clock) so very short times can't be accurately used.
 *
 * @param us Delay in microseconds. At 28MHz the maximum possible value is 153391810 (153.4 seconds). WARNING: Limit is not checked to save processing time!.
 * CYCCNT is a 32 bit timer so maximum time in seconds is 2^32*(1/CORE_CLOCK)+CALC_CYCLES
 * Small values can't be accurately used since the function itself takes 121 clock cycles (11us at 11MHz clock)
 *
 */
__STATIC_INLINE void
delayUs (uint32_t us)
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

/**************************************************************************//**
 * @brief
 *    Configure SysTick to interrupt every millisecond (necessary for Delay())
 *****************************************************************************/
void initSysTick (void)
{
  while (SysTick_Config (CMU_ClockFreqGet (cmuClock_CORE) / 1000))
    {
    }
}

void tourne (unsigned char sens)
{
  static signed char nb_led = 9;
  char i;
  if (nb_led == 9)
    {
      for (i = 12; i < 19; i++)
        {
          LCD_SegmentSet (0, i, true);
        }
    }
  LCD_SegmentSet (0, nb_led + 12, true);

  if (sens)
    {
      nb_led++;
      if (nb_led >= 8)
        nb_led = 0;
      LCD_SegmentSet (0, nb_led + 12, false);
    }
  else
    {
      nb_led--;
      if (nb_led < 0)
        nb_led = 8;
      LCD_SegmentSet (0, nb_led + 12, false);

    }
}
/**************************************************************************//**
 * @brief
 *    Main function
 *****************************************************************************/
int main (void)
{
  long result;
  long tare;
  // Initialization
  CHIP_Init ();
  CMU_ClockSelectSet (cmuClock_HF, cmuSelect_HFXO);
  /* Enable GPIO in CMU */
  CMU_ClockEnable (cmuClock_GPIO, true);
  GPIO_PinModeSet (gpioPortB, 9, gpioModeInput, 0);
  GPIO_PinModeSet (gpioPortE, 2, gpioModePushPull, 0);
  short i = 10;
  unsigned char rot;
  float couple;
  char message[8];
  SegmentLCD_Init (false);
  HX711_config ();
  initSysTick (); // SysTick is used to time the delay
  // delay(500);

  // Wait for the BURTC to wake us up
  //EMU_EnterEM4();
  //SegmentLCD_AllOn();

  SegmentLCD_AllOff ();
  sprintf (message, "Welcome");
  SegmentLCD_Write (message);
  delay (2000);
  sprintf (message, "ENISE");
  SegmentLCD_Write (message);
  delay (2000);
  sprintf (message, "Lab 343");
  SegmentLCD_Write (message);
  delay (2000);
  sprintf (message, "ELEC Xg");
  SegmentLCD_Write (message);
  delay (2000);
  // tare au debut
  tare=0;
  rot = 0;
  for (i = 0; i < 10; i++)
    {
    tare += HX711_Read (GAIN);
    }
  tare = tare / 10;
  delay(500);
  //fin tare

  while (1)
    {
      result=0;
      // Test Appuie bouon tare
      if (GPIO_PinInGet (gpioPortB, 9))
        {
          // pas appuie bouton tire a VCC
          //GPIO_PinOutSet(gpioPortE,2);
        }
      else
        {
          //Appuie sur Tare
          // on doit stocker la valeur en sortie du CAN ( hx711)
          //GPIO_PinOutClear(gpioPortE,2);
          tare=0;
          for (i = 0; i < 10; i++)
            {
              tare += HX711_Read (GAIN);
            }
          tare = tare / 10;
        }
      for (i = 0; i < 10; i++)
        {
          result += HX711_Read (GAIN);
        }
      result=result/10;
      //SegmentLCD_Number(i--); // Display reset counter to the LCD
      //SegmentLCD_Symbol(LCD_SYMBOL_MINUS,1);
      couple = (float) (result-tare) * COEFF_COUPLE;
      if (couple < 0)
        {
          couple = -couple;
          rot=1;
          SegmentLCD_Symbol (LCD_SYMBOL_MINUS, 1);
        }
      else
        {
          rot=0;
          SegmentLCD_Symbol (LCD_SYMBOL_MINUS, 0);
        }
      sprintf (message, "%.2fNm", couple);
      SegmentLCD_Write (message);
      if ((result-tare) != 0)
        {
          tourne (rot);
        }
      //  LCD_SegmentSet(col, round++, true);
      //  if  (round==8) round=0;
  //    CMU_ClockFreqGet (cmuClock_CORE);
      //delayUs(1000000);
      //sl_udelay_wait (1000000);
      //result = HX711_Read (GAIN);
   	 delay(1);
    }
}

