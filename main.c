#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <strings.h>

#include <applibs/log.h>
#include <applibs/gpio.h>


/*#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#include <applibs/log.h>
#include <applibs/gpio.h>*/
#include "hbridge.h"


_MotorState MotorState;

// File descriptors - initialized to invalid value
static GPIO_Id PinEnable = -1;
static GPIO_Id PinForward = -1;
static GPIO_Id PinReverse = -1;

//// static GPIO_Id gpioId = -1;

bool ExitNow = false;
bool UsingHW = false;

static int controllerOpenPin(int pin, int mode)
{
    int fd;

    if (mode == 0)
        fd = GPIO_OpenAsInput(pin);
    else
        fd = GPIO_OpenAsOutput(pin, GPIO_OutputMode_PushPull, GPIOPinLow);
    return fd;
}

static bool controllerRead(int pin)
{
    GPIO_Value_Type outValue;
    switch (pin)
    {
    case pinEn:
        GPIO_GetValue(PinEnable, &outValue);
        break;
    case pinFwd:
        GPIO_GetValue(PinForward, &outValue);
        break;
    case pinRev:
        GPIO_GetValue(PinReverse, &outValue);
        break;;
    }
    return (outValue == GPIOPinHigh ? true : false);
}


static void controllerWrite(int pin, int val)
{
    GPIO_Value_Type pinState = (val == 1) ? GPIOPinHigh :GPIOPinLow;
    switch (pin)
    {
    case pinEn:
        GPIO_SetValue(PinEnable, pinState);
        break;
    case pinFwd:
        GPIO_SetValue(PinForward, pinState);
        break;
    case pinRev:
        GPIO_SetValue(PinReverse, pinState);
        break;;
    }
}

static void InitHBridge()
{
    MotorState.EnState = false;
    MotorState.FwdState = false;
    MotorState.RevState = false;
    int result;
    result = -1;
    result++;
    Log_Debug("Let's control a DC motor!\n");

    PinEnable = controllerOpenPin(pinEn, modeOut);
    Log_Debug("GPIO pin enabled for use (Output:Enable): {%d}\n", pinEn);
    PinForward = controllerOpenPin(pinRev, modeOut);
    Log_Debug("GPIO pin enabled for use (Output:Reverse): {%d}\n", pinRev);
    PinReverse = controllerOpenPin(pinFwd, modeOut);
    Log_Debug("GPIO pin enabled for use (Output:Forward): {%d}\n", pinFwd);

    controllerWrite(pinEn, PinLow);
    controllerWrite(pinFwd,PinLow);
    controllerWrite(pinRev,PinLow);

    MotorState.FwdState = controllerRead(pinFwd);
    MotorState.RevState = controllerRead(pinRev);
    MotorState.EnState = controllerRead(pinEn);
    UsingHW = true;
    Log_Debug("Hardware is available\n");
}

void Msg(char* msg, const char* msgIn)
{
    memset(msg, 0, 30);
    strncpy(msg, msgIn, strlen(msgIn));
}

#define fwdOK "Motor going fwd"
#define fwdNOK    "Fwd but not enabled"
#define revOK "Motor going rev"
#define revNOK    "Rev but not enabled"
#define brakeOK "Motor is braked"
#define brakeNOK    "Braked but not enabled"
#define motorEnabled "Motor Enabled"
#define motorDisabled "Motor Disabled"


void  RunMotor(char cmd)
{

    //BYTE buffer[1024];

    bool exitNow = false;
    char MsgOut[30];

    //Nb: if pinFwd=pinRev hi or lo then its brake
    strncpy(MsgOut, "CMD OK\0", 7);
    if (cmd == '\0')
    {
        return; // MsgOut;
    }
    else
    {
        switch (cmd)
        {
        case '0':
            controllerWrite(pinFwd, PinLow);
            Log_Debug("    Pin: {%d} State: (%d)\n", pinFwd, controllerRead(pinFwd));
            break;
        case '1':
            controllerWrite(pinFwd, PinHigh);
            Log_Debug("    Pin: {%d} State: (%d)\n", pinFwd, controllerRead(pinFwd));
            break;
        case '2':
            controllerWrite(pinRev, PinLow);
            Log_Debug("    Pin: {%d} State: (%d)\n", pinRev, controllerRead(pinRev));
            break;
        case '3':
            controllerWrite(pinRev, PinHigh);
            Log_Debug("    Pin: {%d} State: (%d)\n", pinRev, controllerRead(pinRev));
            break;
        case '4':
            controllerWrite(pinEn, PinLow);
            Log_Debug("    Pin: {%d} State: (%d)\n", pinEn, controllerRead(pinEn));
            break;
        case '5':
            controllerWrite(pinEn, PinHigh);
            Log_Debug("    Pin: {%d} State: (%d)\n", pinEn, controllerRead(pinEn));
            break;

        case 'F': //Forward
                    //Fwd: Take action so as to eliminate undesirable intermediate state/s
            if (MotorState.FwdState && MotorState.RevState)
            {
                //Is braked (hi)
                controllerWrite(pinRev, PinLow);
                Log_Debug("    Pin: {%d} State: (%d)\n", pinRev, controllerRead(pinRev));
            }
            else if (!MotorState.FwdState && MotorState.RevState)
            {
                //Is Rev. Brake first
                controllerWrite(pinRev, PinLow);
                controllerWrite(pinFwd, PinHigh);
                Log_Debug("    Pin: {%d} State: (%d)\n", pinFwd, controllerRead(pinFwd));
                Log_Debug("    Pin: {%d} State: (%d)\n", pinRev, controllerRead(pinRev));
            }
            else if (!MotorState.FwdState && !MotorState.RevState)
            {
                //Is braked (lo)
                controllerWrite(pinFwd, PinHigh);
                Log_Debug("    Pin: {%d} State: (%d)\n", pinFwd, controllerRead(pinFwd));
            }
            else if (MotorState.FwdState && !MotorState.RevState)
            {
                //Is fwd
            }
            if (MotorState.EnState)
                Msg(MsgOut, fwdOK);
            else
                Msg(MsgOut, fwdNOK);
            break;
        case 'R': // Reverse
            if (MotorState.FwdState && MotorState.RevState)
            {
                //Is braked (hi)
                controllerWrite(pinFwd, PinLow);
                Log_Debug("    Pin: {%d} State: (%d)\n", pinFwd, controllerRead(pinFwd));
            }
            else if (!MotorState.FwdState && MotorState.RevState)
            {
                //Is reverse
            }
            else if (!MotorState.FwdState && !MotorState.RevState)
            {
                //Is braked (lo)
                controllerWrite(pinRev, PinHigh);
                Log_Debug("    Pin: {%d} State: (%d)\n", pinRev, controllerRead(pinRev));
            }
            else if (MotorState.FwdState && !MotorState.RevState)
            {
                //Is fwd: Brake first
                controllerWrite(pinFwd, PinLow);
                controllerWrite(pinRev, PinHigh);
                Log_Debug("    Pin: {%d} State: (%d)\n", pinFwd, controllerRead(pinFwd));
                Log_Debug("    Pin: {%d} State: (%d)\n", pinRev, controllerRead(pinRev));
            }
            if (MotorState.EnState)
                Msg(MsgOut, revOK);
            else 
                Msg(MsgOut,revNOK);
            break;
        case 'B': //Brake
            if (MotorState.FwdState && MotorState.RevState)
            {
                //Is braked (hi)
            }
            else if (!MotorState.FwdState && MotorState.RevState)
            {
                //Is Rev: Brake lo
                controllerWrite(pinRev, PinLow);
                Log_Debug("    Pin: {%d} State: (%d)\n", pinRev, controllerRead(pinRev));
            }
            else if (!MotorState.FwdState && !MotorState.RevState)
            {
                //Is braked (lo)
            }
            else if (MotorState.FwdState && !MotorState.RevState)
            {
                //Is fwd: Brake lo
                controllerWrite(pinFwd, PinLow);
                Log_Debug("    Pin: {%d} State: (%d)\n", pinFwd, controllerRead(pinFwd));
            }
            if (MotorState.EnState)
                Msg(MsgOut, brakeOK);
            else
                Msg(MsgOut, brakeNOK);
            break;
        case 'E': //Enable
            controllerWrite(pinEn, PinHigh);
            Log_Debug("    Pin: {%d} State: (%d)\n", pinEn, controllerRead(pinEn));
            Msg(MsgOut, motorEnabled);
            break;
        case 'D': //Disable
            controllerWrite(pinEn, PinLow);
            Log_Debug("    Pin: {%d} State: (%d)\n", pinEn, controllerRead(pinEn));
            Msg(MsgOut, motorDisabled);
            break;
        case 'Q':
            Msg(MsgOut, "Exiting");
            ExitNow = true;
            break;
            //case default:
            //    MsgOut = "Invalid command";
            //    break;
        }

        MotorState.FwdState = (bool)controllerRead(pinFwd);
        MotorState.RevState = (bool)controllerRead(pinRev);
        MotorState.EnState = (bool)controllerRead(pinEn);
    }

    Log_Debug("        Device: %s\n", MsgOut);
}



int main(void)
{
    // This minimal Azure Sphere app repeatedly toggles GPIO 9, which is the green channel of RGB
    // LED 1 on the MT3620 RDB.
    // If your device exposes different GPIOs, you might need to change this value. For example,
    // to run the app on a Seeed mini-dev kit, change the GPIO from 9 to 7 in the call to
    // GPIO_OpenAsOutput and in the app_manifest.json to blink its LED. Check with your hardware
    // manufacturer to determine which GPIOs are available.
    // Use this app to test that device and SDK installation succeeded that you can build,
    // deploy, and debug an app with Visual Studio, and that you can deploy an app over the air,
    // per the instructions here: https://docs.microsoft.com/azure-sphere/quickstarts/qs-overview
    //
    // It is NOT recommended to use this as a starting point for developing apps; instead use
    // the extensible samples here: https://github.com/Azure/azure-sphere-samples
    Log_Debug(
        "\nVisit https://github.com/Azure/azure-sphere-samples for extensible samples to use as a "
        "starting point for full applications.\n");

    // Change this GPIO number and the number in app_manifest.json if required by your hardware.
    int fd = GPIO_OpenAsOutput(9, GPIO_OutputMode_PushPull, GPIO_Value_High);
    if (fd < 0) {
        Log_Debug(
            "Error opening GPIO: %s (%d). Check that app_manifest.json includes the GPIO used.\n",
            strerror(errno), errno);
        return -1;
    }

    int fd2 = GPIO_OpenAsOutput(15, GPIO_OutputMode_PushPull, GPIO_Value_High);
    if (fd2 < 0) {
        Log_Debug(
            "Error opening GPIO: %s (%d). Check that app_manifest.json includes the GPIO used.\n",
            strerror(errno), errno);
        return -1;
    }

    InitHBridge();
    const struct timespec sleepTime = { 1, 0 };

    for (int i = 0; i < 1; i++) {
        RunMotor('1');
        nanosleep(&sleepTime, NULL);

        RunMotor('3');
        nanosleep(&sleepTime, NULL);

        RunMotor('5');
        nanosleep(&sleepTime, NULL);


        RunMotor('0');
        nanosleep(&sleepTime, NULL);

        RunMotor('2');
        nanosleep(&sleepTime, NULL);

        RunMotor('4');
        nanosleep(&sleepTime, NULL);
    }

   



    const struct timespec sleepTime2 = { 3, 0 };
    GPIO_SetValue(fd, GPIO_Value_Low);
    GPIO_SetValue(fd2, GPIO_Value_Low);
    RunMotor('D');
    nanosleep(&sleepTime2, NULL);
    RunMotor('F');
    nanosleep(&sleepTime2, NULL);
    RunMotor('R');
    nanosleep(&sleepTime2, NULL);
    RunMotor('B');
    nanosleep(&sleepTime2, NULL);



 

    GPIO_SetValue(fd, GPIO_Value_Low);
    GPIO_SetValue(fd2, GPIO_Value_Low);
        while (true) {

            RunMotor('E');
            nanosleep(&sleepTime, NULL);
            RunMotor('F');
            GPIO_SetValue(fd, GPIO_Value_High);
            GPIO_SetValue(fd2, GPIO_Value_Low);
            nanosleep(&sleepTime2, NULL);
            RunMotor('R');
            GPIO_SetValue(fd, GPIO_Value_Low);
            GPIO_SetValue(fd2, GPIO_Value_High);
            nanosleep(&sleepTime2, NULL);
            RunMotor('B');
            GPIO_SetValue(fd, GPIO_Value_Low);
            GPIO_SetValue(fd2, GPIO_Value_Low);
            nanosleep(&sleepTime2, NULL);
            RunMotor('D');
            nanosleep(&sleepTime2, NULL);
        }
    
}
