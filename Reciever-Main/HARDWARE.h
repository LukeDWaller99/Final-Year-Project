#ifndef __HARDWARE_H__
#define __HARDWARE_H__

#include <PinNames.h>

// pins for nRF24L01
#define MOSI    PB_5
#define MISO    PB_4
#define SCK     PB_3
#define CSN     PA_4
#define CE      PB_13
#define IRQ     PB_12

// BLDC motor pins
#define FWD_LHS_MOTOR   PE_5
#define FWD_RHS_MOTOR   PE_6
#define REV_LHS_MOTOR   PF_8
#define REV_RHS_MOTOR   PF_9

#endif