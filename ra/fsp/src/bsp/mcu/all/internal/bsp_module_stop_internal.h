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

#ifndef BSP_MODULE_STOP_INTERNAL_H
#define BSP_MODULE_STOP_INTERNAL_H

/** Common macro for FSP header files. There is also a corresponding FSP_FOOTER macro at the end of this file. */
FSP_HEADER

#if BSP_MCU_GROUP_RA2A2
 #define BSP_MSTP_REG_FSP_IP_AGT(channel)    R_MSTP->MSTPCRD

/* MSTP Bits are messy on RA2A2.
 * Ch 0-1: MSTPD[ 3: 2] (AGTW0, AGTW1)
 * Ch 2-3: MSTPD[19:18] (AGT0, AGT1)
 * Ch 4-5: MSTPD[ 1: 0] (AGT2, AGT3)
 * Ch 6-9: MSTPD[10: 7] (AGT4, AGT5, AGT6, AGT7)
 */
 #define BSP_MSTP_BIT_FSP_IP_AGT(channel)    (1U <<                                                             \
                                              ((channel < BSP_FEATURE_AGT_AGTW_CHANNEL_COUNT)                   \
                                               ? (3U - channel)                                                 \
                                               : ((channel < BSP_FEATURE_AGT_AGTW_CHANNEL_COUNT + 2U)           \
                                                  ? (19U - channel + BSP_FEATURE_AGT_AGTW_CHANNEL_COUNT)        \
                                                  : ((channel < BSP_FEATURE_AGT_AGTW_CHANNEL_COUNT + 4U)        \
                                                     ? (1U - channel + BSP_FEATURE_AGT_AGTW_CHANNEL_COUNT + 2U) \
                                                     : (10U - channel + BSP_FEATURE_AGT_AGTW_CHANNEL_COUNT + 4U)))));

#endif

/** Common macro for FSP header files. There is also a corresponding FSP_HEADER macro at the top of this file. */
FSP_FOOTER

#endif
