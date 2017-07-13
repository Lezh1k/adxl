#include <stdint.h>

/*----------------------------------------------------------------------------
  Exception / Interrupt Handler Function Prototype
 *----------------------------------------------------------------------------*/
typedef void( *pFunc )( void );
extern void SystemInit (void);
extern int main(void);

void IntDefaultHandler(void);                           /* Default empty handler */
void ResetIntHandler(void);                             /* Reset Handler */


/*----------------------------------------------------------------------------
  User Initial Stack & Heap
 *----------------------------------------------------------------------------*/
#ifndef __STACK_SIZE
#define	__STACK_SIZE  0x00000400
#endif
static uint8_t stack[__STACK_SIZE] __attribute__ ((aligned(8), used, section(".stack")));

#ifndef __HEAP_SIZE
#define	__HEAP_SIZE   0x00000C00
#endif
#if __HEAP_SIZE > 0
static uint8_t heap[__HEAP_SIZE]   __attribute__ ((aligned(8), used, section(".heap")));
#endif

extern uint32_t _etext, _data, _edata, _bss, _ebss, __stack_top;

// Simple gcc- and g++-compatible C runtime init
extern uint32_t __init_array_start;
extern uint32_t __init_array_end;


/*----------------------------------------------------------------------------
  Exception / Interrupt Handler
 *----------------------------------------------------------------------------*/
/* Cortex-M0+ Processor Exceptions */
void NMI_Handler         (void) __attribute__ ((weak, alias("IntDefaultHandler")));
void HardFault_Handler   (void) __attribute__ ((weak, alias("IntDefaultHandler")));
void SVC_Handler         (void) __attribute__ ((weak, alias("IntDefaultHandler")));
void PendSV_Handler      (void) __attribute__ ((weak, alias("IntDefaultHandler")));
void SysTick_Handler     (void) __attribute__ ((weak, alias("IntDefaultHandler")));

/* ARMCM0plus Specific Interrupts */
void WDT_IRQHandler      (void) __attribute__ ((weak, alias("IntDefaultHandler")));
void RTC_IRQHandler      (void) __attribute__ ((weak, alias("IntDefaultHandler")));
void TIM0_IRQHandler     (void) __attribute__ ((weak, alias("IntDefaultHandler")));
void TIM2_IRQHandler     (void) __attribute__ ((weak, alias("IntDefaultHandler")));
void MCIA_IRQHandler     (void) __attribute__ ((weak, alias("IntDefaultHandler")));
void MCIB_IRQHandler     (void) __attribute__ ((weak, alias("IntDefaultHandler")));
void UART0_IRQHandler    (void) __attribute__ ((weak, alias("IntDefaultHandler")));
void UART1_IRQHandler    (void) __attribute__ ((weak, alias("IntDefaultHandler")));
void UART2_IRQHandler    (void) __attribute__ ((weak, alias("IntDefaultHandler")));
void UART4_IRQHandler    (void) __attribute__ ((weak, alias("IntDefaultHandler")));
void AACI_IRQHandler     (void) __attribute__ ((weak, alias("IntDefaultHandler")));
void CLCD_IRQHandler     (void) __attribute__ ((weak, alias("IntDefaultHandler")));
void ENET_IRQHandler     (void) __attribute__ ((weak, alias("IntDefaultHandler")));
void USBDC_IRQHandler    (void) __attribute__ ((weak, alias("IntDefaultHandler")));
void USBHC_IRQHandler    (void) __attribute__ ((weak, alias("IntDefaultHandler")));
void CHLCD_IRQHandler    (void) __attribute__ ((weak, alias("IntDefaultHandler")));
void FLEXRAY_IRQHandler  (void) __attribute__ ((weak, alias("IntDefaultHandler")));
void CAN_IRQHandler      (void) __attribute__ ((weak, alias("IntDefaultHandler")));
void LIN_IRQHandler      (void) __attribute__ ((weak, alias("IntDefaultHandler")));
void I2C_IRQHandler      (void) __attribute__ ((weak, alias("IntDefaultHandler")));
void CPU_CLCD_IRQHandler (void) __attribute__ ((weak, alias("IntDefaultHandler")));
void UART3_IRQHandler    (void) __attribute__ ((weak, alias("IntDefaultHandler")));
void SPI_IRQHandler      (void) __attribute__ ((weak, alias("IntDefaultHandler")));

/*----------------------------------------------------------------------------
  Exception / Interrupt Vector table
 *----------------------------------------------------------------------------*/
const pFunc __Vectors[] __attribute__ ((section(".vectors"))) = {
  /* Cortex-M0+ Exceptions Handler */
  (pFunc)((uint32_t)&__stack_top),           /*      Initial Stack Pointer     */
  ResetIntHandler,                            /*      Reset Handler             */
  NMI_Handler,                              /*      NMI Handler               */
  HardFault_Handler,                        /*      Hard Fault Handler        */
  0,                                        /*      Reserved                  */
  0,                                        /*      Reserved                  */
  0,                                        /*      Reserved                  */
  0,                                        /*      Reserved                  */
  0,                                        /*      Reserved                  */
  0,                                        /*      Reserved                  */
  0,                                        /*      Reserved                  */
  SVC_Handler,                              /*      SVCall Handler            */
  0,                                        /*      Reserved                  */
  0,                                        /*      Reserved                  */
  PendSV_Handler,                           /*      PendSV Handler            */
  SysTick_Handler,                          /*      SysTick Handler           */
  /* External interrupts */
  WDT_IRQHandler,                           /*  0:  Watchdog Timer            */
  RTC_IRQHandler,                           /*  1:  Real Time Clock           */
  TIM0_IRQHandler,                          /*  2:  Timer0 / Timer1           */
  TIM2_IRQHandler,                          /*  3:  Timer2 / Timer3           */
  MCIA_IRQHandler,                          /*  4:  MCIa                      */
  MCIB_IRQHandler,                          /*  5:  MCIb                      */
  UART0_IRQHandler,                         /*  6:  UART0 - DUT FPGA          */
  UART1_IRQHandler,                         /*  7:  UART1 - DUT FPGA          */
  UART2_IRQHandler,                         /*  8:  UART2 - DUT FPGA          */
  UART4_IRQHandler,                         /*  9:  UART4 - not connected     */
  AACI_IRQHandler,                          /* 10: AACI / AC97                */
  CLCD_IRQHandler,                          /* 11: CLCD Combined Interrupt    */
  ENET_IRQHandler,                          /* 12: Ethernet                   */
  USBDC_IRQHandler,                         /* 13: USB Device                 */
  USBHC_IRQHandler,                         /* 14: USB Host Controller        */
  CHLCD_IRQHandler,                         /* 15: Character LCD              */
  FLEXRAY_IRQHandler,                       /* 16: Flexray                    */
  CAN_IRQHandler,                           /* 17: CAN                        */
  LIN_IRQHandler,                           /* 18: LIN                        */
  I2C_IRQHandler,                           /* 19: I2C ADC/DAC                */
  0,                                        /* 20: Reserved                   */
  0,                                        /* 21: Reserved                   */
  0,                                        /* 22: Reserved                   */
  0,                                        /* 23: Reserved                   */
  0,                                        /* 24: Reserved                   */
  0,                                        /* 25: Reserved                   */
  0,                                        /* 26: Reserved                   */
  0,                                        /* 27: Reserved                   */
  CPU_CLCD_IRQHandler,                      /* 28: Reserved - CPU FPGA CLCD   */
  0,                                        /* 29: Reserved - CPU FPGA        */
  UART3_IRQHandler,                         /* 30: UART3    - CPU FPGA        */
  SPI_IRQHandler                            /* 31: SPI Touchscreen - CPU FPGA */
};

static inline void
crt0(void) {
  uint32_t *src, *dest;

  // copy the data section
  src  = &_etext;
  dest = &_data;
  while (dest < &_edata)
    *(dest++) = *(src++);

  // blank the bss section
  while (dest < &_ebss)
    *(dest++) = 0;

  // call C++ constructors
  dest = &__init_array_start;
  while (dest < &__init_array_end)
    (*(void(**)(void)) dest++)();
}

/*----------------------------------------------------------------------------
  Reset Handler called on controller reset
 *----------------------------------------------------------------------------*/
__attribute__ ((section(".after_vectors"), naked))
void
ResetIntHandler(void) {
  SystemInit();
  crt0();
  main();
  while (1) ; // hang if main returns
}

// Processor ends up here if an unexpected interrupt occurs or a specific
// handler is not present in the application code.
__attribute__ ((section(".after_vectors")))
void
IntDefaultHandler(void) {
  while(1) ;
}
