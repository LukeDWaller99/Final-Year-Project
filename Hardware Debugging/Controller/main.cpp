/**
    @file Main.cpp
    Main file for Final Year Project - Luke Waller
**/

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <mbed.h>
#include <nRF24L01P.h>
#include <string>
#include "Buzzer.h"
#include "Callback.h"
#include "DigitalIn.h"
#include "DigitalOut.h"
#include "EventQueue.h"
#include "PinNames.h"
#include "ThisThread.h"
#include "HARDWARE.h"
#include "TRASMISSION_CODES.h"

#define TRANSFER_SIZE   5
#define DEFAULT_PIPE    0 // set the defauly pipe for the nRF24L01

#define MULTIPLYING_FACTOR  2.0f
#define POT_OFFSET          1.0f

#define RADIO_QUEUE_LENGTH 32


// MOSI, MISO, SCK, CNS, CE, IRQ - must be an interrupt pin 6 
nRF24L01P nRF24L01(MOSI, MISO, SCK, CSN, CE, IRQ);

InterruptIn Btn_1(BTN1), Btn_2(BTN2), Btn_3(BTN3), SW_1(SW1);        
DigitalOut led_1(LED_1);                                             
AnalogIn L_Pitch(L_PITCH), L_Roll(L_ROLL), R_Pitch(R_PITCH), R_Roll(R_ROLL);   
Buzzer buzzer(BUZZER);

    char rxData[TRANSFER_SIZE] = {RX_DATA};
    char txData[TRANSFER_SIZE] = {TX_DATA};

struct stickData{
    char fwdLeftPitch[TRANSFER_SIZE] = {LEFT_PITCH_FWD}, 
        fwdLeftRoll[TRANSFER_SIZE] = {LEFT_ROLL_FWD}, 
        fwdRightPitch[TRANSFER_SIZE] = {RIGHT_PITCH_FWD}, 
        fwdRightRoll[TRANSFER_SIZE] = {RIGHT_ROLL_FWD}, 
        fwdLeftPitchTemp[TRANSFER_SIZE], 
        fwdLeftRollTemp[TRANSFER_SIZE], 
        fwdRightPitchTemp[TRANSFER_SIZE], 
        fwdRightRollTemp[TRANSFER_SIZE],
        revLeftPitch[TRANSFER_SIZE] = {LEFT_PITCH_REV}, 
        revLeftRoll[TRANSFER_SIZE] = {LEFT_ROLL_REV}, 
        revRightPitch[TRANSFER_SIZE] = {RIGHT_PITCH_REV}, 
        revRightRoll[TRANSFER_SIZE] = {RIGHT_PITCH_REV}, 
        revLeftPitchTemp[TRANSFER_SIZE], 
        revLeftRollTemp[TRANSFER_SIZE], 
        revRightPitchTemp[TRANSFER_SIZE], 
        revRightRollTemp[TRANSFER_SIZE];
};

struct btnData{
    char BTN1_PRESSED[TRANSFER_SIZE] = {BUTTON_1},
        BTN2_PRESSED[TRANSFER_SIZE] = {BUTTON_2}, 
        BTN3_PRESSED[TRANSFER_SIZE] = {BUTTON_3}; 
};


char txSW1[TRANSFER_SIZE] = {SW1_ON};


int txDataCnt = TRANSFER_SIZE, rxDataCnt = 0;

Thread LEDThread, ButtonThread, PotThread, RadioThread;

Mutex PotLock;

float potVals[6], oldPotVals[6];


void PotMethod();

void toggleLEDs();
void ButtonThreadMethod();
void Btn_1IRQ();
void Btn_2IRQ();
void Btn_3IRQ();
void Btn_4IRQ();
void SW_1RisingIRQ();
void SW_1FallingIRQ();
void SW_2RisingIRQ();
void SW_2FallingIRQ();

// create an array of outputs for the leds for the output
int main() {

    nRF24L01.powerUp();

// Display the (default) setup of the nRF24L01+ chip
    printf("nRF24L01 Frequency    : %d MHz\n",  nRF24L01.getRfFrequency() );
    printf("nRF24L01 Output power : %d dBm\n",  nRF24L01.getRfOutputPower());
    printf("nRF24L01 Data Rate    : %d kbps\n", nRF24L01.getAirDataRate());
    printf("nRF24L01 TX Address   : 0x%010llX\n", nRF24L01.getTxAddress());
    printf("nRF24L01 RX Address   : 0x%010llX\n", nRF24L01.getRxAddress());

    nRF24L01.setTransferSize(TRANSFER_SIZE);
 
    nRF24L01.setTransmitMode();

    nRF24L01.enable();

    printf("%s\n", txData);
    nRF24L01.write(txData, 0 , TRANSFER_SIZE);

    led_1 = 1;
    // buzzer = 1;
    // wait_us(1000000);
    // buzzer = 0;

    PotThread.start(PotMethod);
    LEDThread.start(toggleLEDs);
    ButtonThread.start(ButtonThreadMethod);
    Btn_1.rise(Btn_1IRQ);
    Btn_2.rise(Btn_2IRQ);
    Btn_3.rise(Btn_3IRQ);
    SW_1.rise(SW_1RisingIRQ);
    SW_1.fall(SW_1FallingIRQ);

    while (true) {
        sleep();
    }
}

void PotMethod(){

    stickData tx;

    int oldLeftPitchVal = 0;
    int newLeftPitchVal = 0;

    int oldLeftRollVal = 0;
    int newLeftRollVal = 0;

    int oldRightPitchVal = 0;
    int newRightPitchVal = 0;

    int oldRightRollVal = 0;
    int newRightRollVal = 0;

    char tempThrottleChar[2];

    while(true){

        PotLock.trylock_for(10ms);
        potVals[0] = ((L_Pitch.read() * MULTIPLYING_FACTOR) - POT_OFFSET) * 9;
        potVals[1] = ((L_Roll.read() * MULTIPLYING_FACTOR) - POT_OFFSET) * 9;
        potVals[2] = ((R_Pitch.read() * MULTIPLYING_FACTOR) - POT_OFFSET) * 9;
        potVals[3] = ((R_Roll.read() * MULTIPLYING_FACTOR) - POT_OFFSET) * 9;

        newLeftPitchVal     = round(potVals[0]);
        newLeftRollVal      = round(potVals[1]);
        newRightPitchVal    = round(potVals[2]);
        newRightRollVal     = round(potVals[3]);

        // for left pitch values
        if (oldLeftPitchVal != newLeftPitchVal) {
            if(newLeftPitchVal > 0){ // forwards
                newLeftPitchVal = abs(newLeftPitchVal);
                sprintf(tempThrottleChar, "%d", newLeftPitchVal);
                tx.fwdLeftPitch[3] = tempThrottleChar[1];
                nRF24L01.write(tx.fwdLeftPitch, 0, TRANSFER_SIZE);
            } else if(newLeftPitchVal < 0){ // reverse 
                newLeftPitchVal = abs(newLeftPitchVal);
                sprintf(tempThrottleChar, "%d", newLeftPitchVal);
                tx.revLeftPitch[3] = tempThrottleChar[1];
                nRF24L01.write(tx.revLeftPitch, 0, TRANSFER_SIZE);
            } else { // equal to zero
                newLeftPitchVal = 0;
                sprintf(tempThrottleChar, "%d", newLeftPitchVal);
                tx.fwdLeftPitch[3] = tempThrottleChar[1];
                nRF24L01.write(tx.fwdLeftPitch, 0, TRANSFER_SIZE);
            }
        }

        // for left roll values
        if (oldLeftRollVal != newLeftRollVal) {
            if(newLeftRollVal > 0){ // forwards
                newLeftRollVal = abs(newLeftRollVal);
                sprintf(tempThrottleChar, "%d", newLeftRollVal);
                tx.fwdLeftRoll[3] = tempThrottleChar[1];
                nRF24L01.write(tx.fwdLeftRoll, 0, TRANSFER_SIZE);
            } else if(newLeftRollVal < 0){ // reverse 
                newLeftRollVal = abs(newLeftRollVal);
                sprintf(tempThrottleChar, "%d", newLeftRollVal);
                tx.revLeftRoll[3] = tempThrottleChar[1];
                nRF24L01.write(tx.revLeftRoll, 0, TRANSFER_SIZE);
            } else { // equal to zero
                newLeftPitchVal = 0;
                sprintf(tempThrottleChar, "%d", newLeftRollVal);
                tx.fwdLeftRoll[3] = tempThrottleChar[1];
                nRF24L01.write(tx.fwdLeftRoll, 0, TRANSFER_SIZE);
            }
        }

        // for right pitch values
        if (oldRightPitchVal != newRightPitchVal) {
            if(newRightPitchVal > 0){ // forwards
                newRightPitchVal = abs(newRightPitchVal);
                sprintf(tempThrottleChar, "%d", newRightPitchVal);
                tx.fwdRightPitch[3] = tempThrottleChar[1];
                nRF24L01.write(tx.fwdRightPitch, 0, TRANSFER_SIZE);
            } else if(newRightPitchVal < 0){ // reverse 
                newRightPitchVal = abs(newRightPitchVal);
                sprintf(tempThrottleChar, "%d", newRightPitchVal);
                tx.revRightPitch[3] = tempThrottleChar[1];
                nRF24L01.write(tx.revRightPitch, 0, TRANSFER_SIZE);
            } else { // equal to zero
                newRightPitchVal = 0;
                sprintf(tempThrottleChar, "%d", newRightPitchVal);
                tx.fwdRightPitch[3] = tempThrottleChar[1];
                nRF24L01.write(tx.fwdRightPitch, 0, TRANSFER_SIZE);
            }
        }

        // for right roll values
        if (oldRightRollVal != newRightRollVal) {
            if(newRightRollVal > 0){ // forwards
                newRightRollVal = abs(newRightRollVal);
                sprintf(tempThrottleChar, "%d", newRightRollVal);
                tx.fwdRightRoll[3] = tempThrottleChar[1];
                nRF24L01.write(tx.fwdRightRoll, 0, TRANSFER_SIZE);
            } else if(newRightRollVal < 0){ // reverse 
                newRightRollVal = abs(newLeftRollVal);
                sprintf(tempThrottleChar, "%d", newRightRollVal);
                tx.revRightRoll[3] = tempThrottleChar[1];
                nRF24L01.write(tx.revRightRoll, 0, TRANSFER_SIZE);
            } else { // equal to zero
                newRightPitchVal = 0;
                sprintf(tempThrottleChar, "%d", newRightRollVal);
                tx.fwdRightRoll[3] = tempThrottleChar[1];
                nRF24L01.write(tx.fwdRightRoll, 0, TRANSFER_SIZE);
            }
        }

        oldLeftPitchVal = newLeftPitchVal;
        oldLeftRollVal = newLeftRollVal;
        oldRightPitchVal = newRightPitchVal;
        oldRightRollVal = newRightRollVal;

        PotLock.unlock();
        ThisThread::sleep_for(10ms);
    }
}

void ButtonThreadMethod(){

    btnData tx;

    printf("Button Thread Started\n");
    while (true) {
        buzzer = 0;
        ThisThread::flags_wait_any(0x7fffffff, false);
        int flag = ThisThread::flags_get();
        ThisThread::flags_clear(0x7fffffff);
        printf("Flag = %d\n", flag);
        switch (flag) {
        case 1:
            printf("Button 1 pressed\n");
            printf("%s\n", tx.BTN1_PRESSED);
            nRF24L01.write(tx.BTN1_PRESSED, 0, TRANSFER_SIZE);
            break;
        case 2: 
            printf("Button 2 pressed\n");
            printf("%s\n", tx.BTN2_PRESSED);
            nRF24L01.write(tx.BTN2_PRESSED, 0, TRANSFER_SIZE);
            break;
        case 4:
            printf("Button 3 pressed\n");
            printf("%s\n", tx.BTN3_PRESSED);
            nRF24L01.write(tx.BTN3_PRESSED, 0, TRANSFER_SIZE);
            break;
        case 8:
            printf("Switch 1 ON\n");
            txSW1[3] = '1';
            nRF24L01.write(txSW1);
            break;
        case 16:
            printf("Switch 1 OFF\n");
            txSW1[3] = '0';
            nRF24L01.write(txSW1);
            break;
        default:
            printf("Error!\n");
            break;
        }
        flag = 0;
    }
}

void Btn_1IRQ(){
    ButtonThread.flags_set(1);
}

void Btn_2IRQ(){
    ButtonThread.flags_set(2);
}

void Btn_3IRQ(){
    ButtonThread.flags_set(4);
}

void SW_1RisingIRQ(){
    ButtonThread.flags_set(8);
}

void SW_1FallingIRQ(){
    ButtonThread.flags_set(16);
}


void toggleLEDs(){
    printf("LED Thread Started \n");
    while (true) {
    led_1 = !led_1;
    ThisThread::sleep_for(1s);
    }
}