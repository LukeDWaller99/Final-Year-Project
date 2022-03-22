/**
    @file Main.cpp
    Main file for Final Year Project - Luke Waller
**/

#include "InterruptIn.h"
#include <cstdio>
#include <mbed.h>
#include <nRF24L01P.h>
#include <HARDWARE.h>

#define TRANSFER_SIZE   5
#define DEFAULT_PIPE    0

char txData[TRANSFER_SIZE];
char rxData[TRANSFER_SIZE];

nRF24L01P nRF24L01(MOSI, MISO, SCK, CSN, CE);

InterruptIn SerialInterruptIn(IRQ);

void SerialIRQ();

int led = 0;

int main() {

    SerialInterruptIn.fall(SerialIRQ);

    printf("Starting Board...\n");

    nRF24L01.powerUp();

    // Display the (default) setup of the nRF24L01 chip
    printf("nRF24L01 Frequency    : %d MHz\n",  nRF24L01.getRfFrequency() );
    printf("nRF24L01 Output power : %d dBm\n",  nRF24L01.getRfOutputPower() );
    printf("nRF24L01 Data Rate    : %d kbps\n", nRF24L01.getAirDataRate() );
    printf("nRF24L01 TX Address   : 0x%010llX\n", nRF24L01.getTxAddress() );
    printf("nRF24L01 RX Address   : 0x%010llX\n", nRF24L01.getRxAddress() );

    nRF24L01.setTransferSize(TRANSFER_SIZE);
    nRF24L01.enableAutoAcknowledge();
    nRF24L01.enable();

    printf("Starting Handshake...\n");

    nRF24L01.setReceiveMode();

    printf("Receive Mode...\n");


    while(true){
        if(led == 1){
            nRF24L01.read(rxData);
            printf("rxData: %s\n", rxData);
            led = 0;
        }
    }
}

void SerialIRQ(){
    led = 1;
}
