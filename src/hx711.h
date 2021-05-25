/*
 * hx711.h
 *
 *  Created on: 4 févr. 2021
 *      Author: xgaltier
 */

#ifndef SRC_HX711_H_
#define SRC_HX711_H_

#include "em_gpio.h"
#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_burtc.h"
#include "em_rmu.h"
#include "em_gpio.h"
#include "segmentlcd.h"
#include "udelay.h"

typedef char byte;


/*********************
 * CONFIG PIN FOR HX711
 *
 **********************/
#define PD_SCK gpioPortD,0
#define DOUT gpioPortD,1

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

bool digitalRead(GPIO_Port_TypeDef port, unsigned char pin);
void digitalWrite(GPIO_Port_TypeDef port, unsigned char pin, bool value);

bool HX711_Is_Ready();
long HX711_Read(unsigned char gain);
void HX711_config();

#endif /* SRC_HX711_H_ */
