#include "control.h"

void Delay(void) {
    for (volatile int i = 0; i < 200; i++);
    //HAL_Delay(1);
}

uint8_t TransmitReceive(uint8_t data) {
    uint8_t received = 0;

    for (int i = 7; i >= 0; i--) {
        if (data & (1 << i))
            SetMOSI();
        else
            ResetMOSI();

        Delay();

        SetSCK();
        Delay();

        if (ReadMISO())
            received |= (1 << i);

        ResetSCK();
        Delay();
    }

    return received;
}

void Transmit(uint8_t data) {
    for (int i = 7; i >= 0; i--) {
        if (data & (1 << i))
            SetMOSI();
        else
            ResetMOSI();

        SetSCK();
        Delay();
        ResetSCK();
        Delay();
    }
}

uint8_t Receive(void){
    uint8_t received = 0;

    for (int i = 7; i >= 0; i--) {
        SetSCK();
        Delay();

        if (ReadMISO()) {received |= (1 << i);}

        ResetSCK();
        Delay();
    }

    return received;

}