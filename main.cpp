#include "mbed.h"
#include "RotaryEncoder.h"

#define FINGER_COEFFICIENT -1.3
#define WRIST_X_COEFFICIENT 1.2
#define WRIST_Y_COEFFICIENT 1.3
#define WRIST_Z_COEFFICIENT -0.75

#define FINGER_CORRECTION 0x00
#define WRIST_X_CORRECTION 0x80
#define WRIST_Y_CORRECTION 0x00
#define WRIST_Z_CORRECTION 0x80

RawSerial PC(USBTX, USBRX, 115200);
RawSerial Shishi(PA_9, PA_10, 230400);
RotaryEncoder RE_finger(PB_5, PB_4);
RotaryEncoder RE_wristX(PA_3, PA_1);
RotaryEncoder RE_wristY(PA_7, PA_6);
RotaryEncoder RE_wristZ(PA_8, PA_11);
DigitalIn LiftUp(PB_0);
DigitalIn LiftDown(PB_7);
DigitalIn AUTO(PA_12);
DigitalIn CurtainOpen(PB_1);
DigitalIn CurtainClose(PF_0);
DigitalIn RE_lock(PA_4);
DigitalIn HelperHeight(PA_0);

int extended_information = 0x00;
int helper_sign = 0x00;
int last_finger_sign;
int last_wristX_sign;
int last_wristY_sign;

int check_sum;

int main()
{
    LiftUp.mode(PullUp);
    LiftDown.mode(PullUp);
    AUTO.mode(PullUp);
    CurtainOpen.mode(PullUp);
    CurtainClose.mode(PullUp);
    
    while (true) {
        check_sum = 0x00;
        Shishi.putc((char)0xFF);

        wait_us(1500);

        extended_information = (!HelperHeight.read() << 7) + (!AUTO.read() << 6) + (!LiftUp.read() << 5) + (!LiftDown.read() << 4) + (!CurtainOpen.read() << 3) + (!CurtainClose.read() << 2) + (RE_lock.read() << 1); 
        Shishi.putc((char)extended_information);
        check_sum += extended_information;

        wait_us(1500);

        int RE_finger_sign = (int)(RE_finger.Get_Count() * FINGER_COEFFICIENT + FINGER_CORRECTION);
    
        if(RE_finger_sign > 0xFE) {
            RE_finger_sign = 0xFE;
        }
        else if(RE_finger_sign < 0) {
            RE_finger_sign = 0x00;
        }
        Shishi.putc(RE_finger_sign);
        check_sum += RE_finger_sign;

        wait_us(1500);

        int RE_wristY_sign = (int)((RE_wristY.Get_Count() - 20) * WRIST_Y_COEFFICIENT + WRIST_Y_CORRECTION);

        if(RE_wristY_sign > 0xFE) {
            RE_wristY_sign = 0xFE;
        }
        else if(RE_wristY_sign < 0x00) {
            RE_wristY_sign = 0x00;
        }
        Shishi.putc(RE_wristY_sign);
        check_sum += RE_wristY_sign;
        
        wait_us(1500);

        int RE_wristX_sign = (int)(RE_wristX.Get_Count() * WRIST_X_COEFFICIENT * ((RE_wristY_sign > 0x80) ? (RE_wristY_sign > 0xE8 ? 0.6 : 0.85) : 1.0) + WRIST_X_CORRECTION);

        if(RE_wristX_sign > 0xFE) {
            RE_wristX_sign = 0xFE;
        }
        else if(RE_wristX_sign < 0) {
            RE_wristX_sign = 0x00;
        }
        Shishi.putc(RE_wristX_sign);
        check_sum += RE_wristX_sign;
        
        wait_us(1500);

        int RE_wristZ_sign = (int)(RE_wristZ.Get_Count() * WRIST_Z_COEFFICIENT * ((RE_wristY_sign > 0x80) ? 0.5 : 1.0) + WRIST_Z_CORRECTION);

        if(RE_wristZ_sign > 0xFE) {
            RE_wristZ_sign = 0xFE;
        }
        else if(RE_wristZ_sign < 0x00) {
            RE_wristZ_sign = 0x00;
        }

        Shishi.putc(RE_wristZ_sign);
        check_sum += RE_wristZ_sign;

        wait_us(1500);

        if(check_sum % 0x100 == 0xFF) {
            Shishi.putc(0xFE);
        }
        else {
            Shishi.putc(check_sum % 0x100);
        }

        PC.printf("\033[H");
        PC.printf("0x %02X %02X %02X %02X %02X %02X", extended_information, RE_finger_sign, RE_wristX_sign, RE_wristY_sign, RE_wristZ_sign, (check_sum % 0x100));
        // PC.printf("AUTO :    %d\r\nUP   :    %d\r\nDOWN :    %d\r\nOPEN :    %d\r\nCLOSE:    %d\r\nLOCK :    %d\r\nHELP :    %d\r\nN-RX : %4d\r\nN-RY : %4d\r\nCHIN : %4d",!AUTO.read(),!LiftUp.read(),!LiftDown.read(),!CurtainOpen.read(),!CurtainClose.read(),RE_lock.read(),!HelperHeight.read(),RE_wristX.Get_Count(),RE_wristY.Get_Count(),RE_finger.Get_Count());
        wait_us(1500);
    }
}
