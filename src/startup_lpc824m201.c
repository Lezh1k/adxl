#include <stdint.h>

/*----------------------------------------------------------------------------
  Exception / Interrupt Handler Function Prototype
 *----------------------------------------------------------------------------*/
typedef void( *pFunc )( void );
extern void SystemInit (void);
extern int main(void);
extern uint32_t _etext, _data, _edata, _bss, _ebss, __stack_top;

// Simple gcc- and g++-compatible C runtime init
extern uint32_t __init_array_start;
extern uint32_t __init_array_end;

/*----------------------------------------------------------------------------
  User Initial Stack & Heap
 *----------------------------------------------------------------------------*/
//#ifndef __STACK_SIZE
//#define	__STACK_SIZE  0x00000400
//#endif
//static uint8_t g_stack[__STACK_SIZE] __attribute__ ((aligned(8), used, section(".stack")));

//#ifndef __HEAP_SIZE
//#define	__HEAP_SIZE   0x00000C00
//#endif
//#if __HEAP_SIZE > 0
//static uint8_t g_heap[__HEAP_SIZE]   __attribute__ ((aligned(8), used, section(".heap")));
//#endif


/*----------------------------------------------------------------------------
  Exception / Interrupt Handler
 *----------------------------------------------------------------------------*/
/* Cortex-M0+ Processor Exceptions */
void ResetIntHandler     (void); /* Reset Handler */
void NMI_Handler         (void) {  while(1) ; }
void HardFault_Handler   (void) {  while(1) ; }
void SVC_Handler         (void) {  while(1) ; }
void PendSV_Handler      (void) {  while(1) ; }
void SysTick_Handler     (void) {  while(1) ; }

/* ARMCM0plus Specific Interrupts */
extern void SPI0_IRQHandler() ;                    // SPI0 controller
extern void SPI1_IRQHandler() ;                    // SPI1 controller
extern void UART0_IRQHandler() ;                   // UART0
extern void UART1_IRQHandler() ;                   // UART1
extern void UART2_IRQHandler() ;                   // UART2
extern void I2C1_IRQHandler() ;                    // I2C1 controller
extern void I2C0_IRQHandler() ;                    // I2C0 controller
extern void SCT_IRQHandler() ;                     // Smart Counter Timer
extern void MRT_IRQHandler() ;                     // Multi-Rate Timer
extern void CMP_IRQHandler() ;                     // Comparator
extern void WDT_IRQHandler() ;                     // PIO1 (0:11)
extern void BOD_IRQHandler() ;                     // Brown Out Detect
extern void FLASH_IRQHandler() ;                   // FLASH controller
extern void WKT_IRQHandler() ;                     // Wakeup timer
extern void ADC_SEQA_IRQHandler() ;                // ADC SEQA
extern void ADC_SEQB_IRQHandler() ;                // ADC SEQB
extern void ADC_THCMP_IRQHandler() ;               // ADC Threashold Compare
extern void ADC_OVR_IRQHandler() ;                 // ADC Overrun
extern void DMA_IRQHandler() ;                     // DMA controller
extern void I2C2_IRQHandler() ;                    // I2C2 controller
extern void I2C3_IRQHandler() ;                    // I2C3 controller
extern void PININT0_IRQHandler() ;                 // PIO INT0
extern void PININT1_IRQHandler() ;                 // PIO INT1
extern void PININT2_IRQHandler() ;                 // PIO INT2
extern void PININT3_IRQHandler() ;                 // PIO INT3
extern void PININT4_IRQHandler() ;                 // PIO INT4
extern void PININT5_IRQHandler() ;                 // PIO INT5
extern void PININT6_IRQHandler() ;                 // PIO INT6
extern void PININT7_IRQHandler() ;                 // PIO INT7

extern void __valid_user_code_checksum() __attribute__ ((weak));
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
  __valid_user_code_checksum,          /*      Reserved  ???             */
  0,                                        /*      Reserved                  */
  0,                                        /*      Reserved                  */
  0,                                        /*      Reserved                  */
  SVC_Handler,                              /*      SVCall Handler            */
  0,                                        /*      Reserved                  */
  0,                                        /*      Reserved                  */
  PendSV_Handler,                           /*      PendSV Handler            */
  SysTick_Handler,                          /*      SysTick Handler           */
  /* External interrupts */
  SPI0_IRQHandler,                    // SPI0 controller
  SPI1_IRQHandler,                    // SPI1 controller
  0,                                  // Reserved
  UART0_IRQHandler,                   // UART0
  UART1_IRQHandler,                   // UART1
  UART2_IRQHandler,                   // UART2
  0,                                  // Reserved
  I2C1_IRQHandler,                    // I2C1 controller
  I2C0_IRQHandler,                    // I2C0 controller
  SCT_IRQHandler,                     // Smart Counter Timer
  MRT_IRQHandler,                     // Multi-Rate Timer
  CMP_IRQHandler,                     // Comparator
  WDT_IRQHandler,                     // PIO1 (0:11)
  BOD_IRQHandler,                     // Brown Out Detect
  FLASH_IRQHandler,                   // FLASH controller
  WKT_IRQHandler,                     // Wakeup timer
  ADC_SEQA_IRQHandler,                // ADC SEQA
  ADC_SEQB_IRQHandler,                // ADC SEQB
  ADC_THCMP_IRQHandler,               // ADC Threashold Compare
  ADC_OVR_IRQHandler,                 // ADC Overrun
  DMA_IRQHandler,                     // DMA controller
  I2C2_IRQHandler,                    // I2C2 controller
  I2C3_IRQHandler,                    // I2C3 controller
  0,                                  // Reserved
  PININT0_IRQHandler,                 // PIO INT0
  PININT1_IRQHandler,                 // PIO INT1
  PININT2_IRQHandler,                 // PIO INT2
  PININT3_IRQHandler,                 // PIO INT3
  PININT4_IRQHandler,                 // PIO INT4
  PININT5_IRQHandler,                 // PIO INT5
  PININT6_IRQHandler,                 // PIO INT6
  PININT7_IRQHandler,                 // PIO INT7
};
//////////////////////////////////////////////////////////////////////////

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
}
//////////////////////////////////////////////////////////////////////////

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
//////////////////////////////////////////////////////////////////////////
