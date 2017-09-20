#ifndef ARMUART_H
#define ARMUART_H

#include <stdint.h>

// ArmUart(int num, int baud, char *rxBuffer, int rxSize, char *txBuffer, int
// txSize); ~ArmUart();
void armUart(char *rxBuffer, int32_t rxSize, char *txBuffer, int32_t txSize);
void armUartDelete();

void armUartPutch(char sym);
void armUartSendBreak();
void armUartInterrupt0();  // rxReady, txready, txidle
void armUartTick();        // tickrx, ticktx

int32_t armUartGetch();  //-2 = break
uint8_t armUartKbhit();
int32_t armUartReadyCount();  //не учитывает размер бряков и т.п. то есть
                              //покажет больше чем есть на самом деле
int32_t armUartRxFreeTicks();

#endif  // ARMUART_H
