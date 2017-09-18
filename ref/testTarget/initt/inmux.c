/*
* @brief
* This file is used to configure the Input Mux module.
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

#include "chip.h"    /* LPC824M201JDH20 Peripheral Registers */

void InputMux_Init()
{
    LPC_INMUX->SCT0_INMUX[0] = 15;    /*  */
    LPC_INMUX->SCT0_INMUX[1] = 15;    /*  */
    LPC_INMUX->SCT0_INMUX[2] = 15;    /*  */
    LPC_INMUX->SCT0_INMUX[3] = 15;    /*  */
    LPC_DMATRIGMUX->DMA_ITRIG_INMUX[0] = 15;    /*  */
    LPC_DMATRIGMUX->DMA_ITRIG_INMUX[1] = 15;    /*  */
    LPC_DMATRIGMUX->DMA_ITRIG_INMUX[2] = 15;    /*  */
    LPC_DMATRIGMUX->DMA_ITRIG_INMUX[3] = 15;    /*  */
    LPC_DMATRIGMUX->DMA_ITRIG_INMUX[4] = 15;    /*  */
    LPC_DMATRIGMUX->DMA_ITRIG_INMUX[5] = 15;    /*  */
    LPC_DMATRIGMUX->DMA_ITRIG_INMUX[6] = 15;    /*  */
    LPC_DMATRIGMUX->DMA_ITRIG_INMUX[7] = 15;    /*  */
    LPC_DMATRIGMUX->DMA_ITRIG_INMUX[8] = 15;    /*  */
    LPC_DMATRIGMUX->DMA_ITRIG_INMUX[9] = 15;    /*  */
    LPC_DMATRIGMUX->DMA_ITRIG_INMUX[10] = 15;    /*  */
    LPC_DMATRIGMUX->DMA_ITRIG_INMUX[11] = 15;    /*  */
    LPC_DMATRIGMUX->DMA_ITRIG_INMUX[12] = 15;    /*  */
    LPC_DMATRIGMUX->DMA_ITRIG_INMUX[13] = 15;    /*  */
    LPC_DMATRIGMUX->DMA_ITRIG_INMUX[14] = 15;    /*  */
    LPC_DMATRIGMUX->DMA_ITRIG_INMUX[15] = 15;    /*  */
    LPC_DMATRIGMUX->DMA_ITRIG_INMUX[16] = 15;    /*  */
    LPC_DMATRIGMUX->DMA_ITRIG_INMUX[17] = 15;    /*  */
    LPC_INMUX->DMA_INMUX_INMUX[0] = 31;    /*  */
    LPC_INMUX->DMA_INMUX_INMUX[1] = 31;    /*  */

}

 /**********************************************************************
 **                            End Of File
 **********************************************************************/
