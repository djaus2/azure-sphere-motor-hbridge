#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <time.h>


enum PinMode { modeIn=0,modeOut=1};
enum PinValue { PinLow = 0, PinHigh = 1 };

typedef struct
{
    bool FwdState;
    bool RevState;
    bool EnState;
} _MotorState;


#define pinFwd (2) 
// <- Brd Pin 2           If hi and pinBack is lo motor goes fwd
#define pinRev (3) 
// <- Brd Pin (4)           if hi and pinFwd is lo motor goes back (reverse)
#define pinEn  (0) 
// <- Brd Pin 6           Overall enable/disable  hi/lo




#define StrPinValueLow  "Lo"
#define StrPinValueHigh  "Hi"
#define GPIOPinLow GPIO_Value_Low
#define GPIOPinHigh GPIO_Value_High

static void InitHBridge(void);
static int controllerOpenPin(int pin, int mode);
static bool controllerRead(int pin);
static void controllerWrite(int pin, int val);