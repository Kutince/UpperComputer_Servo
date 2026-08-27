#include "App_Vofa_JustFloat.h"
#include "Com_Vofa_JustFloat.h"
#include "Int_Vofa_JustFloat.h"
#include "Dri_Servo.h"

void App_Vofa_JustFloat(void)
{
    VOFA_SetChannel(0,(float)Angle);
    VOFA_SendChannels(1);
}