/***********************************************************************************************************************
 * Copyright [2020-2023] Renesas Electronics Corporation and/or its affiliates.  All Rights Reserved.
 *
 * This software and documentation are supplied by Renesas Electronics America Inc. and may only be used with products
 * of Renesas Electronics Corp. and its affiliates ("Renesas").  No other uses are authorized.  Renesas products are
 * sold pursuant to Renesas terms and conditions of sale.  Purchasers are solely responsible for the selection and use
 * of Renesas products and Renesas assumes no liability.  No license, express or implied, to any intellectual property
 * right is granted by Renesas. This software is protected under all applicable laws, including copyright laws. Renesas
 * reserves the right to change or discontinue this software and/or this documentation. THE SOFTWARE AND DOCUMENTATION
 * IS DELIVERED TO YOU "AS IS," AND RENESAS MAKES NO REPRESENTATIONS OR WARRANTIES, AND TO THE FULLEST EXTENT
 * PERMISSIBLE UNDER APPLICABLE LAW, DISCLAIMS ALL WARRANTIES, WHETHER EXPLICITLY OR IMPLICITLY, INCLUDING WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND NONINFRINGEMENT, WITH RESPECT TO THE SOFTWARE OR
 * DOCUMENTATION.  RENESAS SHALL HAVE NO LIABILITY ARISING OUT OF ANY SECURITY VULNERABILITY OR BREACH.  TO THE MAXIMUM
 * EXTENT PERMITTED BY LAW, IN NO EVENT WILL RENESAS BE LIABLE TO YOU IN CONNECTION WITH THE SOFTWARE OR DOCUMENTATION
 * (OR ANY PERSON OR ENTITY CLAIMING RIGHTS DERIVED FROM YOU) FOR ANY LOSS, DAMAGES, OR CLAIMS WHATSOEVER, INCLUDING,
 * WITHOUT LIMITATION, ANY DIRECT, CONSEQUENTIAL, SPECIAL, INDIRECT, PUNITIVE, OR INCIDENTAL DAMAGES; ANY LOST PROFITS,
 * OTHER ECONOMIC DAMAGE, PROPERTY DAMAGE, OR PERSONAL INJURY; AND EVEN IF RENESAS HAS BEEN ADVISED OF THE POSSIBILITY
 * OF SUCH LOSS, DAMAGES, CLAIMS OR COSTS.
 **********************************************************************************************************************/

/* Ensure Renesas MCU variation definitions are included to ensure MCU
 * specific register variations are handled correctly. */
#ifndef RENESAS_INTERNAL_H
 #define RENESAS_INTERNAL_H

/** @addtogroup Renesas
 * @{
 */

/** @addtogroup RA
 * @{
 */

 #ifdef __cplusplus
extern "C" {
 #endif

/* TODO: Temporary workaround because compilers are not defining __ARM_ARCH_8_1M_MAIN__ for CM85 parts. */
 #if BSP_CFG_MCU_PART_SERIES == 8
  #undef __ARM_ARCH_8M_MAIN__
  #define __ARM_ARCH_8_1M_MAIN__    1
 #endif

 #if BSP_MCU_GROUP_RA2A2
  #include "R7FA2A2AD.h"
 #elif BSP_MCU_GROUP_RA2E3
  #include "R7FA2E307.h"
 #elif BSP_MCU_GROUP_RA8M1
  #include "R7FA8M1AH.h"
 #elif BSP_MCU_GROUP_RA8D1
  #include "R7FA8D1BH.h"
 #elif BSP_MCU_GROUP_RA8T1
  #include "R7FA8T1AH.h"
 #else
  #warning "Unsupported MCU"
 #endif

 #ifdef __cplusplus
}
 #endif

/** @} */ /* End of group RA */

/** @} */ /* End of group Renesas */

#endif
