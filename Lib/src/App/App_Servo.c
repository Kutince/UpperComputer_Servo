#include "App_Servo.h"
#include "Dri_Servo.h"

void App_ServoContorl(void)
{
    Dri_Servo_GetData();
    Dri_Servo_TimSet(Dri_Servo_ProcessData());
}