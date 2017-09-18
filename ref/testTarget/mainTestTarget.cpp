#include <string.h>
#include "chip.h"
#include "uart/uart.h"
#include "initt/initializer.h"


// ПОЛЕЗНОЕ:
// JLinkGDBServerCL.exe  -device LPC824M201 -speed 12000 -if SWD

// C:\Program Files (x86)\SEGGER\JLink_V618\JLink.exe
//-device LPC824M201 -speed 12000 -if SWD -CommanderScript   %{CurrentProject:NativePath}\jlinkload.jlink

// "c:\Program Files (x86)\SEGGER\JLink_V618\JLink.exe" -device LPC824M201 -speed 12000 -if SWD


#define SYSTICKmicroseconds 1000
#define SYSTICDIVMS (1000/SYSTICKmicroseconds)
#define SYSTICKvalue (30 * SYSTICKmicroseconds  )

volatile unsigned int tickVar = 0;
int valPort = 0;
static unsigned int preTickVar = 0;

extern "C" void SysTick_Handler(void)
{
    if(++preTickVar >= SYSTICDIVMS){
        tickVar++;
        preTickVar = 0;
    }
}

void sleep(int ms)
{
    if(!ms)
        return;
    unsigned int tick0 = tickVar;
    unsigned int tick = tick0 + ms;
    for(;;){
        unsigned int t = tickVar;
        if(t < tick0)
            break;
        if(t > tick)
            break;
    }
}


void setupXTAL()
{

    //SystemInit2();

    Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_SWM);
    Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_IOCON);


    Chip_IOCON_PinSetMode(LPC_IOCON, IOCON_PIO8, PIN_MODE_INACTIVE);
    Chip_IOCON_PinSetMode(LPC_IOCON, IOCON_PIO9, PIN_MODE_INACTIVE);

    Chip_SWM_EnableFixedPin(SWM_FIXED_XTALIN);
    Chip_SWM_EnableFixedPin(SWM_FIXED_XTALOUT);

    volatile int i = 0;

    i+=2;

    Chip_SetupXtalClocking();
    LPC_GPIO_PORT->B[0][14] = 1;

    i++;

}

extern "C" void __vectors_start__();
extern "C" void __vectors_start2__();
extern "C" void ResetISR();
extern "C" void (* const g_pfnVectors[])(void);
extern "C" void (* const g_pfnVectors2[])(void);

int main()
{
    // Read clock settings and update SystemCoreClock variable
    SystemCoreClockUpdate();

    LPC_SYSCTL->SYSMEMREMAP = 2;
    SCB->VTOR = 0x2000;

    //Chip_UART_Init(LPC_USART0);

    SwitchMatrix_Init();

    IOCON_Init();

    InputMux_Init();

    Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_SWM);
    //Chip_GPIO_Init(LPC_GPIO_PORT);

    //Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 0, 14);

    // TODO: insert code here

    setupXTAL();

    //SCB->VTOR = (uint32_t)(g_pfnVectors2);
    LPC_SYSCTL->SYSMEMREMAP = 2;
    SCB->VTOR = 0x2000;



    SystemCoreClockUpdate();

    SysTick_Config(SYSTICKvalue);   //30000 = 1ms = 1kHz, 3000 = 10kHz, 60 - 500kHz = ПРЕДЕЛ, порядок скорости вызова int ~ 1us;

    LPC_SYSCTL->SYSMEMREMAP = 2;
    SCB->VTOR = 0x2000;


    __enable_irq();




    Uart::u0();

    u->puts((uintptr_t)(&g_pfnVectors2) == 0x2000 ? ((uintptr_t)(&__vectors_start2__) == 0x2000 ? "*" : "+") : "-");
    Uart::u0()->puts("Privet!\r\n");
    //sleep(10);
    u->printfn("Попытка запуска. last full recompile time is %s %s",__DATE__,__TIME__);
    u->printfn("Vector=%X, VS2=%X, reset=%X. Vs=%X, Vs2=%X",
               &__vectors_start__, &__vectors_start2__, &ResetISR, g_pfnVectors, g_pfnVectors2);
    u->puts("wait\r\n");

    sleep(300);
    u->puts("start.");


    u->puts(".end\r\n");
    for(;;);


    return 0;
}
