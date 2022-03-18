/**
    @file Main.cpp
    Main file for Final Year Project - Luke Waller
**/

#include <mbed.h>
#include <nRF24L01P.h>
#include <HARDWARE.h>

#define TRANSFER_SIZE   5
#define DEFAULT_PIPE    0

nRF24L01P nRF24L01(MOSI, MISO, SCK, CSN, CE, IRQ);

int main() {

    printf("Starting Board...\n");

    nRF24L01.powerUp();

    // Display the (default) setup of the nRF24L01 chip
    printf("nRF24L01 Frequency    : %d MHz\n",  nRF24L01.getRfFrequency() );
    printf("nRF24L01 Output power : %d dBm\n",  nRF24L01.getRfOutputPower() );
    printf("nRF24L01 Data Rate    : %d kbps\n", nRF24L01.getAirDataRate() );
    printf("nRF24L01 TX Address   : 0x%010llX\n", nRF24L01.getTxAddress() );
    printf("nRF24L01 RX Address   : 0x%010llX\n", nRF24L01.getRxAddress() );

    nRF24L01.setTransferSize(TRANSFER_SIZE);

    printf("Starting Handshake...\n");

}
