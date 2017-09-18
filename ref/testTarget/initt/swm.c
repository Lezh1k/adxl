/*
* @brief
* This file is used to configure the  SwitchMatrix module.
*
* @note
* Copyright(C) NXP Semiconductors, 2017
* All rights reserved.
*
* @par
* Software that is described herein is for illustrative purposes only
* which provides customers with programming information regarding the
* LPC products.  This software is supplied "AS IS" without any warranties of
* any kind, and NXP Semiconductors and its licensor disclaim any and
* all warranties, express or implied, including all implied warranties of
* merchantability, fitness for a particular purpose and non-infringement of
* intellectual property rights.  NXP Semiconductors assumes no responsibility
* or liability for the use of the software, conveys no license or rights under any
* patent, copyright, mask work right, or any other intellectual property rights in
* or to any products. NXP Semiconductors reserves the right to make changes
* in the software without notification. NXP Semiconductors also makes no
* representation or warranty that such application will be suitable for the
* specified use without further testing or modification.
*
* @par
* Permission to use, copy, modify, and distribute this software and its
* documentation is hereby granted, under NXP Semiconductors' and its
* licensor's relevant copyrights in the software, without fee, provided that it
* is used in conjunction with NXP Semiconductors microcontrollers.  This
* copyright, permission, and disclaimer notice must appear in all copies of
* this code.
*/

#include "chip.h"    /* LPCOpen  Peripheral Registers for LPC824M201JDH20 */

void SwitchMatrix_Init()
{
       /* Enable the clock to the Switch Matrix */
       Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_SWM);

       /* Pin Assign 8 bit Configuration */
       /* U0_TXD */
       /* U0_RXD */
       LPC_SWM->PINASSIGN[0] = 0xffff0004UL;
       /* SPI0_SCK */
       LPC_SWM->PINASSIGN[3] = 0x0dffffffUL;
       /* SPI0_MOSI */
       /* SPI0_MISO */
       /* SPI0_SSEL0 */
       LPC_SWM->PINASSIGN[4] = 0xff0c0f01UL;

       /* Pin Assign 1 bit Configuration */
       /* SWCLK */
       /* SWDIO */
       /* XTALIN */
       /* XTALOUT */
       /* RESET */
       /* I2C0_SDA */
       /* I2C0_SCL */
       /* ADC_9 */
       LPC_SWM->PINENABLE0 = 0xffbfe60fUL;

}

 /**********************************************************************
 **                            End Of File
 **********************************************************************/
