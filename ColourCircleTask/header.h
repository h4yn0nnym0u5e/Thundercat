
/*
#define TFT_CTP_I2C Wire1 
/*/
#define TFT_CTP_I2C Master1 
//*/
#include <arduino_freertos.h>
#include <initGT911.h>

//=================================================
template<typename T> 
T constrain(T v, T l, T u)
{
    T result = v;
    if (v<l) result = l;
    if (v>u) result = u;
    return result;
}

//=================================================
#define SER_TERM Serial

#define TASK_LIST \
    TASK_LIST_ENTRY(GT911touch) \
    TASK_LIST_ENTRY(MainLCD)

#define COUNT_OF(a) (int)(sizeof a / sizeof a[0])

namespace freertos
{
extern TaskHandle_t g_yield_task;
}

#define IDLE_PIN 2

//=================================================
extern TaskHandle_t handleGT911touch;
extern void initGT911touch(void);

extern void setIdlePin(bool);
extern TaskHandle_t handleMainLCD;
