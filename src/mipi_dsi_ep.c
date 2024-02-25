/***********************************************************************************************************************
 * File Name    : mipi_dsi_ep.c
 * Description  : Contains data structures and functions setup LCD used in hal_entry.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * DISCLAIMER
 * This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
 * other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
 * applicable laws, including copyright laws.
 * THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
 * THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
 * EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
 * SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS
 * SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
 * this software. By using this software, you agree to the additional terms and conditions found by accessing the
 * following link:
 * http://www.renesas.com/disclaimer
 *
 * Copyright (C) 2023 Renesas Electronics Corporation. All rights reserved.
 ***********************************************************************************************************************/
 
#include "RTE_Components.h"
#include "gt911.h"
#include "mipi_dsi_ep.h"
#include "r_mipi_dsi.h"
#include "hal_data.h"
#include "common_utils.h"
#include "perf_counter.h"

#if defined(RTE_CMSIS_RTOS2)
#   include "cmsis_os2.h"
#endif

#if defined(RTE_Acceleration_Arm_2D)
#   include "arm_2d_helper.h"
#   include "arm_2d_scenes.h"

#   include "arm_2d_disp_adapters.h"
#   ifdef RTE_Acceleration_Arm_2D_Extra_Benchmark
#       include "arm_2d_benchmark.h"
#   endif
#endif

#if defined(RTE_Compiler_EventRecorder) || defined(RTE_CMSIS_View_EventRecorder)
#   include <EventRecorder.h>
#endif

#if defined(RTE_GRAPHICS_LVGL)
#   include "lvgl.h"
#   include "demos/lv_demos.h"

#   include "lv_port_disp_template.h"
#   include "lv_port_indev_template.h"
#endif

/*******************************************************************************************************************//**
 * @addtogroup mipi_dsi_ep
 * @{
 **********************************************************************************************************************/

/* User defined functions */
static void display_draw (uint32_t * framebuffer);
static uint8_t mipi_dsi_set_display_time (void);
static uint8_t process_input_data(void);
void handle_error (fsp_err_t err,  const char * err_str);
void touch_screen_reset(void);
static fsp_err_t wait_for_mipi_dsi_event (mipi_dsi_phy_status_t event);
static void mipi_dsi_ulps_enter(void);
static void mipi_dsi_ulps_exit(void);

/* Variables to store resolution information */
uint16_t g_hz_size, g_vr_size;
/* Variables used for buffer usage */
uint32_t g_buffer_size, g_hstride;
uint32_t * gp_single_buffer = NULL;
uint32_t * gp_double_buffer = NULL;
uint32_t * gp_frame_buffer  = NULL;
uint8_t read_data              = RESET_VALUE;
uint16_t period_msec           = RESET_VALUE;
volatile mipi_dsi_phy_status_t g_phy_status;
timer_info_t timer_info = { .clock_frequency = RESET_VALUE, .count_direction = RESET_VALUE, .period_counts = RESET_VALUE };
volatile bool g_vsync_flag, g_message_sent, g_ulps_flag, g_irq_state, g_timer_overflow  = RESET_FLAG;

/* This table of commands was adapted from sample code provided by FocusLCD
 * Page Link: https://focuslcds.com/product/4-5-tft-display-capacitive-tp-e45ra-mw276-c/
 * File Link: https://focuslcds.com/content/E45RA-MW276-C_init_code.txt
 */

lcd_table_setting_t g_lcd_init_focuslcd[] =
{
 {6,  {0xFF, 0xFF, 0x98, 0x06, 0x04, 0x01}, MIPI_DSI_CMD_ID_DCS_LONG_WRITE,        MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x08, 0x10},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x21, 0x01},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},

 {2,  {0x30, 0x01},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x31, 0x00},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},

 {2,  {0x40, 0x14},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x41, 0x33},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x42, 0x02},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x43, 0x09},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x44, 0x06},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x50, 0x70},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x51, 0x70},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x52, 0x00},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x53, 0x48},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x60, 0x07},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x61, 0x00},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x62, 0x08},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x63, 0x00},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},

 {2,  {0xa0, 0x00},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0xa1, 0x03},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0xa2, 0x09},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0xa3, 0x0d},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0xa4, 0x06},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0xa5, 0x16},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0xa6, 0x09},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0xa7, 0x08},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0xa8, 0x03},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0xa9, 0x07},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0xaa, 0x06},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0xab, 0x05},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0xac, 0x0d},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0xad, 0x2c},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0xae, 0x26},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0xaf, 0x00},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},

 {2,  {0xc0, 0x00},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0xc1, 0x04},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0xc2, 0x0b},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0xc3, 0x0f},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0xc4, 0x09},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0xc5, 0x18},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0xc6, 0x07},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0xc7, 0x08},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0xc8, 0x05},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0xc9, 0x09},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0xca, 0x07},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0xcb, 0x05},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0xcc, 0x0c},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0xcd, 0x2d},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0xce, 0x28},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0xcf, 0x00},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},

 {6,  {0xFF, 0xFF, 0x98, 0x06, 0x04, 0x06}, MIPI_DSI_CMD_ID_DCS_LONG_WRITE,        MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x00, 0x21},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x01, 0x09},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x02, 0x00},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x03, 0x00},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x04, 0x01},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x05, 0x01},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x06, 0x80},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x07, 0x05},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x08, 0x02},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x09, 0x80},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x0a, 0x00},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x0b, 0x00},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x0c, 0x0a},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x0d, 0x0a},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x0e, 0x00},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x0f, 0x00},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x10, 0xe0},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x11, 0xe4},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x12, 0x04},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x13, 0x00},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x14, 0x00},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x15, 0xc0},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x16, 0x08},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x17, 0x00},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x18, 0x00},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x19, 0x00},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x1a, 0x00},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x1b, 0x00},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x1c, 0x00},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x1d, 0x00},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},

 {2,  {0x20, 0x01},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x21, 0x23},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x22, 0x45},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x23, 0x67},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x24, 0x01},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x25, 0x23},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x26, 0x45},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x27, 0x67},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},

 {2,  {0x30, 0x01},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x31, 0x11},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x32, 0x00},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x33, 0xee},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x34, 0xff},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x35, 0xcb},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x36, 0xda},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x37, 0xad},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x38, 0xbc},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x39, 0x76},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x3a, 0x67},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x3b, 0x22},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x3c, 0x22},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x3d, 0x22},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x3e, 0x22},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x3f, 0x22},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x40, 0x22},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},

 {2,  {0x53, 0x10},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x54, 0x10},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},

 {6,  {0xFF, 0xFF, 0x98, 0x06, 0x04, 0x07}, MIPI_DSI_CMD_ID_DCS_LONG_WRITE,        MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x18, 0x1d},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x26, 0xb2},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x02, 0x77},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0xe1, 0x79},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x17, 0x22},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},

 {6,  {0xFF, 0xFF, 0x98, 0x06, 0x04, 0x00}, MIPI_DSI_CMD_ID_DCS_LONG_WRITE,        MIPI_DSI_CMD_FLAG_LOW_POWER},
 {120, {0},                                 MIPI_DSI_DISPLAY_CONFIG_DATA_DELAY_FLAG,   (mipi_dsi_cmd_flags_t)0},
 {2,  {0x11, 0x00},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE,       MIPI_DSI_CMD_FLAG_LOW_POWER},
 {5,   {0},                                 MIPI_DSI_DISPLAY_CONFIG_DATA_DELAY_FLAG,   (mipi_dsi_cmd_flags_t)0},
 {2,  {0x29, 0x00},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE,       MIPI_DSI_CMD_FLAG_LOW_POWER},
 {2,  {0x3a, 0x70},                         MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_PARAM, MIPI_DSI_CMD_FLAG_LOW_POWER},

 {0x00, {0},                                MIPI_DSI_DISPLAY_CONFIG_DATA_END_OF_TABLE, (mipi_dsi_cmd_flags_t)0},
};

/*******************************************************************************************************************//**
 * @brief      Initialize LCD
 *
 * @param[in]  table  LCD Controller Initialization structure.
 * @retval     None.
 **********************************************************************************************************************/
void mipi_dsi_push_table (lcd_table_setting_t *table)
{
    fsp_err_t err = FSP_SUCCESS;
    lcd_table_setting_t *p_entry = table;

    while (MIPI_DSI_DISPLAY_CONFIG_DATA_END_OF_TABLE != p_entry->cmd_id)
    {
        mipi_dsi_cmd_t msg =
        {
          .channel = 0,
          .cmd_id = p_entry->cmd_id,
          .flags = p_entry->flags,
          .tx_len = p_entry->size,
          .p_tx_buffer = p_entry->buffer,
        };

        if (MIPI_DSI_DISPLAY_CONFIG_DATA_DELAY_FLAG == msg.cmd_id)
        {
            R_BSP_SoftwareDelay (table->size, BSP_DELAY_UNITS_MILLISECONDS);
        }
        else
        {
            g_message_sent = false;
            /* Send a command to the peripheral device */
            err = R_MIPI_DSI_Command (&g_mipi_dsi0_ctrl, &msg);
            handle_error(err, "** MIPI DSI Command API failed ** \r\n");
            /* Wait */
            while (!g_message_sent);
        }
        p_entry++;
    }
}


bool glcd_is_ready(uint_fast8_t layer)
{
    if (false != (bool) (R_GLCDC->GR[layer].VEN_b.PVEN)) {
        return false;
    }
    if (false != (bool) (R_GLCDC->BG.EN_b.VEN)) {
        return false;
    }
    
    return true;
}

#if defined(RTE_CMSIS_Compiler_STDOUT_Custom)
/// Put a character to the stdout
/// \param[in]   ch  Character to output
/// \return the character written, or -1 on write error
int stdout_putchar (int ch)
{
    return (1 != SEGGER_RTT_Write(SEGGER_INDEX, &ch, 1)) ? -1 : ch;
}
#endif


#define GLCD_LANDSCAPE      0

void Disp0_DrawBitmap (uint32_t x, uint32_t y, uint32_t width, uint32_t height, const uint8_t *bitmap) 
{
#if 1
    volatile uint32_t *pwDes = gp_single_buffer + y * g_hz_size + x;
    uint32_t *pwSrc = (uint32_t *)bitmap;
    for (int_fast16_t i = 0; i < height; i++) {
        memcpy ((uint32_t *)pwDes, pwSrc, width * 4);
        //SCB_CleanDCache_by_Addr(pwDes, width * 4);
        pwSrc += width;
        pwDes += g_hz_size;
    }
#else
  uint32_t        i, j;
  uint32_t        dot;
  const uint32_t *ptr_bmp;
  ptr_bmp = ((const uint32_t *)bitmap) + (width * (height - 1));
#if (GLCD_LANDSCAPE != 0)
  dot     = (y * GLCD_WIDTH) + x;
#else
  dot     = ((g_vr_size - x) * g_hz_size) + y;
#endif

  for (i = 0; i < height; i++) {
    for (j = 0; j < width; j++) {
      gp_single_buffer[dot] = *ptr_bmp++;
#if (GLCD_LANDSCAPE != 0)
      dot += 1;
#else
      dot -= g_hz_size;
#endif
    }
#if (GLCD_LANDSCAPE != 0)
    dot +=  GLCD_WIDTH - j;
#else
    dot += (g_hz_size * j) + 1;
#endif
    ptr_bmp -= 2 * width;
  }
#endif
}


#if __DISP0_CFG_ENABLE_3FB_HELPER_SERVICE__
uintptr_t __DISP_ADAPTER0_3FB_FB0_ADDRESS__;
uintptr_t __DISP_ADAPTER0_3FB_FB1_ADDRESS__;
uintptr_t __DISP_ADAPTER0_3FB_FB2_ADDRESS__;


void __disp_adapter0_request_dma_copy(  arm_2d_helper_3fb_t *ptThis,
                                        void *pObj,
                                        uintptr_t pnSource,
                                        uintptr_t pnTarget,
                                        uint32_t nDataItemCount,
                                        uint_fast8_t chDataItemSize)
{
    memcpy((void *)pnTarget, (const void *)pnSource, nDataItemCount * chDataItemSize);

    arm_2d_helper_3fb_report_dma_copy_complete(ptThis);
}

bool __disp_adapter0_request_2d_copy(   arm_2d_helper_3fb_t *ptThis,
                                        void *pObj,
                                        uintptr_t pnSource,
                                        uint32_t wSourceStride,
                                        uintptr_t pnTarget,
                                        uint32_t wTargetStride,
                                        int16_t iWidth,
                                        int16_t iHeight,
                                        uint_fast8_t chBytePerPixel )
{
    ARM_2D_UNUSED(ptThis);
    ARM_2D_UNUSED(pObj);
    assert(iWidth * chBytePerPixel <= wSourceStride);
    assert(iWidth * chBytePerPixel <= wTargetStride);

    /* 2D copy */
    for (int_fast16_t i = 0; i < iHeight; i++) {
        memcpy( (void *)pnTarget, (void *)pnSource, iWidth * chBytePerPixel );

        pnSource += wSourceStride;
        pnTarget += wTargetStride;
    }

    return true;
}

#endif

#if defined(RTE_Acceleration_Arm_2D)
#   include "demos/arm_2d_scene_meter.h"
#   include "demos/arm_2d_scene_watch.h"
#   include "demos/arm_2d_scene_fitness.h"
#   include "demos/arm_2d_scene_audiomark.h"



void scene_meter_loader(void) 
{
    arm_2d_scene_player_set_switching_mode( &DISP0_ADAPTER,
                                            ARM_2D_SCENE_SWITCH_MODE_SLIDE_RIGHT);
    arm_2d_scene_player_set_switching_period(&DISP0_ADAPTER, 500);
    arm_2d_scene_meter_init(&DISP0_ADAPTER);
}

void scene_watch_loader(void) 
{
    arm_2d_scene_watch_init(&DISP0_ADAPTER);
}

void scene_fitness_loader(void) 
{
    arm_2d_scene_player_set_switching_mode( &DISP0_ADAPTER,
                                            ARM_2D_SCENE_SWITCH_MODE_SLIDE_LEFT);
    arm_2d_scene_player_set_switching_period(&DISP0_ADAPTER, 500);
    arm_2d_scene_fitness_init(&DISP0_ADAPTER);
}

void scene_audiomark_loader(void) 
{
    arm_2d_scene_player_set_switching_mode( &DISP0_ADAPTER,
                                            ARM_2D_SCENE_SWITCH_MODE_ERASE_RIGHT);
    arm_2d_scene_player_set_switching_period(&DISP0_ADAPTER, 2000);

    extern
    IMPL_PFB_ON_DRAW(__disp_adapter0_draw_navigation);

    /* register event handler for evtOnDrawNavigation */
    arm_2d_scene_player_register_on_draw_navigation_event_handler(
                    &DISP0_ADAPTER,
                    __disp_adapter0_draw_navigation,
                    NULL);

    arm_2d_scene_audiomark_init(&DISP0_ADAPTER);
}

void scene0_loader(void) 
{
    arm_2d_scene_player_set_switching_mode( &DISP0_ADAPTER,
                                            ARM_2D_SCENE_SWITCH_MODE_FADE_WHITE);
    arm_2d_scene_player_set_switching_period(&DISP0_ADAPTER, 3000);
    
extern void disp_adapter0_navigator_init(void);
    disp_adapter0_navigator_init();

    arm_2d_scene0_init(&DISP0_ADAPTER);
}

void scene1_loader(void) 
{
    arm_2d_scene1_init(&DISP0_ADAPTER);
}

void scene2_loader(void) 
{
    arm_2d_scene_player_set_switching_mode( &DISP0_ADAPTER,
                                            ARM_2D_SCENE_SWITCH_MODE_SLIDE_UP);
    arm_2d_scene_player_set_switching_period(&DISP0_ADAPTER, 4000);
    arm_2d_scene2_init(&DISP0_ADAPTER);
}

void scene3_loader(void) 
{
    arm_2d_scene3_init(&DISP0_ADAPTER);
}

void scene4_loader(void) 
{
    arm_2d_scene_player_set_switching_mode( &DISP0_ADAPTER,
                                            ARM_2D_SCENE_SWITCH_MODE_SLIDE_RIGHT);
    arm_2d_scene_player_set_switching_period(&DISP0_ADAPTER, 500);
    arm_2d_scene4_init(&DISP0_ADAPTER);
}

void scene5_loader(void) 
{
    arm_2d_scene_player_set_switching_mode( &DISP0_ADAPTER,
                                            ARM_2D_SCENE_SWITCH_MODE_FADE_BLACK);
    arm_2d_scene_player_set_switching_period(&DISP0_ADAPTER, 3000);
    arm_2d_scene5_init(&DISP0_ADAPTER);
}


typedef void scene_loader_t(void);

static scene_loader_t * const c_SceneLoaders[] = {
    scene0_loader,
    scene1_loader,
    scene_meter_loader,
    //scene3_loader,
    scene5_loader,
    scene4_loader,
    //scene2_loader,
    scene_fitness_loader,

    scene_audiomark_loader,
};


/* load scene one by one */
void before_scene_switching_handler(void *pTarget,
                                    arm_2d_scene_player_t *ptPlayer,
                                    arm_2d_scene_t *ptScene)
{
    static uint_fast8_t s_chIndex = 0;

    if (s_chIndex >= dimof(c_SceneLoaders)) {
        s_chIndex = 0;
    }
    
    /* call loader */
    c_SceneLoaders[s_chIndex]();
    s_chIndex++;
}


#endif


/*******************************************************************************************************************//**
 * @brief      Start video mode and draw color bands on the LCD screen
 *
 * @param[in]  None.
 * @retval     None.
 **********************************************************************************************************************/
void mipi_dsi_start_display(void)
{
#if defined(RTE_Compiler_EventRecorder) || defined(RTE_CMSIS_View_EventRecorder)
    EventRecorderInitialize(0, 1);
#endif

    SEGGER_RTT_Init();
    
    fsp_err_t err = FSP_SUCCESS;

    /* Get LCDC configuration */
    g_hz_size = (g_display_cfg.input[0].hsize);
    g_vr_size = (g_display_cfg.input[0].vsize);
    g_hstride = (g_display_cfg.input[0].hstride);

    /* Initialize buffer pointers */
    g_buffer_size = (uint32_t) (g_hz_size * g_vr_size * BYTES_PER_PIXEL);
    gp_single_buffer = (uint32_t*) g_display_cfg.input[0].p_base;

#if __DISP0_CFG_ENABLE_3FB_HELPER_SERVICE__
    __DISP_ADAPTER0_3FB_FB0_ADDRESS__ = ((uintptr_t)gp_single_buffer) + g_buffer_size * BYTES_PER_PIXEL * 0;
    __DISP_ADAPTER0_3FB_FB1_ADDRESS__ = ((uintptr_t)gp_single_buffer) + g_buffer_size * BYTES_PER_PIXEL * 1;
    __DISP_ADAPTER0_3FB_FB2_ADDRESS__ = ((uintptr_t)gp_single_buffer) + g_buffer_size * BYTES_PER_PIXEL * 2;
#endif

    /* Double buffer for drawing color bands with good quality */
    gp_double_buffer = gp_single_buffer + g_buffer_size;

    /* Get timer information */
    err = R_GPT_InfoGet (&g_timer0_ctrl, &timer_info);
    /* Handle error */
    handle_error(err, "** GPT InfoGet API failed ** \r\n");

    /* Start video mode */
    err = R_MIPI_DSI_Start (&g_mipi_dsi0_ctrl);
    /* Handle error */
    handle_error(err, "** MIPI DSI Start API failed ** \r\n");

    /* Enable external interrupt */
    err = R_ICU_ExternalIrqEnable(&g_external_irq_ctrl);
    /* Handle error */
    handle_error(err, "** ICU ExternalIrqEnable API failed ** \r\n");

    /* Display color bands on LCD screen */
    display_draw (gp_single_buffer);

    if (glcd_is_ready(DISPLAY_FRAME_LAYER_1)) {
        err = R_GLCDC_BufferChange (&g_display_ctrl, (uint8_t*) gp_single_buffer, DISPLAY_FRAME_LAYER_1);
        handle_error (err, "** GLCD BufferChange API failed ** \r\n");

        /* Wait for a Vsync event */
        g_vsync_flag = RESET_FLAG;
        while (g_vsync_flag);
    }

    printf("EK-RA8D1 Template.\r\n");
#if defined(RTE_GRAPHICS_LVGL)
    /* LVGL Demo */

    lv_init();
    lv_port_disp_init();
    //lv_port_indev_init();


#if LV_USE_DEMO_BENCHMARK

    lv_demo_benchmark();
    
    //lv_demo_benchmark_run_scene(LV_DEMO_BENCHMARK_MODE_RENDER_AND_DRIVER, 26*2-1);      // run scene no 31
#elif LV_USE_DEMO_RENDER
    lv_demo_render(LV_DEMO_RENDER_SCENE_IMAGE_NORMAL, 128);
#elif LV_USE_DEMO_WIDGETS
    lv_demo_widgets();
#elif LV_USE_DEMO_MUSIC
    lv_demo_music();
#endif

    while(1) {
        lv_timer_periodic_handler();
        delay_us(500);
    }

#elif defined(RTE_Acceleration_Arm_2D)
    printf("Arm-2D running on RA8D1\r\n");
    
    arm_2d_init();
    disp_adapter0_init();
    
    //arm_2d_scene0_init(&DISP0_ADAPTER);
#ifdef RTE_Acceleration_Arm_2D_Extra_Benchmark
    arm_2d_run_benchmark();
#else
    arm_2d_scene_player_register_before_switching_event_handler(
            &DISP0_ADAPTER,
            before_scene_switching_handler);
    
    arm_2d_scene_player_switch_to_next_scene(&DISP0_ADAPTER);
#endif

    
#if 0
    while (true)
    {
        uint8_t StatusRegister = RESET_VALUE;
        bool touch_flag = RESET_FLAG;

        /* User selects time to enter ULPS  */
        err = mipi_dsi_set_display_time ();
        handle_error (err, "** mipi_dsi_set_display_time function failed ** \r\n");

        if (g_irq_state)
        {
            g_irq_state = RESET_FLAG;
            /* Get buffer status from gt911 device */
            err = gt911_get_status (&StatusRegister);
            handle_error (err, "** gt911_get_status function failed ** \r\n");

            if (StatusRegister & GT911_BUFFER_STATUS_READY)
            {
                touch_flag = SET_FLAG;

                /* Reset display time when touch is detected */
                err = R_GPT_Reset (&g_timer0_ctrl);
                g_timer_overflow = RESET_FLAG;
                handle_error (err, "** GPT Reset API failed ** \r\n");
            }
        }
        if (g_ulps_flag)
        {
            if (touch_flag)
            {
                /* Exit Ultra-low Power State (ULPS) and turn on the backlight */
                mipi_dsi_ulps_exit();
            }
        }
        else
        {
            if (!g_timer_overflow)
            {
                //disp_adapter0_task();
                //display_draw(gp_single_buffer);
                /* Swap the active framebuffer */
                gp_frame_buffer = (gp_frame_buffer == gp_single_buffer) ? gp_double_buffer : gp_single_buffer;

                /* Display color bands on LCD screen */
                display_draw (gp_frame_buffer);

                /* Now that the framebuffer is ready, update the GLCDC buffer pointer on the next Vsync */
                
                if (glcd_is_ready(DISPLAY_FRAME_LAYER_1)) {
                    err = R_GLCDC_BufferChange (&g_display_ctrl, (uint8_t*) gp_frame_buffer, DISPLAY_FRAME_LAYER_1);
                    handle_error (err, "** GLCD BufferChange API failed ** \r\n");

                    /* Wait for a Vsync event */
                    g_vsync_flag = RESET_FLAG;
                    while (g_vsync_flag);
                }
            }
            else
            {
                /* Enter Ultra-low Power State (ULPS) and turn off the backlight */
                mipi_dsi_ulps_enter();
            }
        }
    }
#else

#ifndef LCD_TARGET_FPS
#   define LCD_TARGET_FPS       60
#endif

    bool bRefreshLCD = false;
    while (1) {
    
        /* lock framerate */
        if (arm_2d_helper_is_time_out(1000 / LCD_TARGET_FPS)) {
            bRefreshLCD = true;
        }
        
        if (bRefreshLCD) {
            if (arm_fsm_rt_cpl == disp_adapter0_task()) {
                bRefreshLCD = false;
            }
        }
    }
#endif
#else

#if defined(__PERF_COUNTER_COREMARK__)
    printf("\r\n Run Coremark...\r\n");
    coremark_main();
#endif
    
    while(1) {
        
    }

#endif
}

/*******************************************************************************************************************//**
 * @brief      User-defined function to draw the current display to a framebuffer.
 *
 * @param[in]  framebuffer   Pointer to frame buffer.
 * @retval     FSP_SUCCESS : Upon successful operation, otherwise: failed.
 **********************************************************************************************************************/
static uint8_t mipi_dsi_set_display_time (void)
{
    fsp_err_t err = FSP_SUCCESS;
    if(APP_CHECK_DATA)
        {
            /* Conversion from input string to integer value */
            read_data = process_input_data();
            switch (read_data)
            {
                /* Set 5 seconds to enter Ultra-Low Power State (ULPS)  */
                case RTT_SELECT_5S:
                {
                    APP_PRINT(MIPI_DSI_INFO_5S);
                    period_msec = GPT_DESIRED_PERIOD_5SEC;
                    break;
                }

                /* Set 15 seconds to enter Ultra-Low Power State (ULPS)  */
                case RTT_SELECT_15S:
                {
                    APP_PRINT(MIPI_DSI_INFO_15S);
                    period_msec = GPT_DESIRED_PERIOD_15SEC;
                    break;
                }

                /* Set 30 seconds to enter Ultra-Low Power State (ULPS)  */
                case RTT_SELECT_30S:
                {
                    APP_PRINT(MIPI_DSI_INFO_30S);
                    period_msec = GPT_DESIRED_PERIOD_30SEC;
                    break;
                }
                /* Stop timer to always display.*/
                case RTT_SELECT_DISABLE_ULPS:
                {
                    APP_PRINT(MIPI_DSI_INFO_DISABLE_ULPS);
                    g_timer_overflow = RESET_FLAG;
                    err = R_GPT_Stop (&g_timer0_ctrl);
                    APP_ERR_RETURN(err, " ** GPT Stop API failed ** \r\n");
                    break;
                }
                default:
                {
                    APP_PRINT("\r\nInvalid Option Selected\r\n");
                    APP_PRINT(MIPI_DSI_MENU);
                    break;
                }
            }

            if (RTT_SELECT_DISABLE_ULPS != read_data)
            {
                /* Calculate the desired period based on the current clock. Note that this calculation could overflow if the
                 * desired period is larger than UINT32_MAX / pclkd_freq_hz. A cast to uint64_t is used to prevent this. */
                uint32_t period_counts = (uint32_t) (((uint64_t) timer_info.clock_frequency * period_msec) / GPT_UNITS_SECONDS);
                /* Set the calculated period. */
                err = R_GPT_PeriodSet (&g_timer0_ctrl, period_counts);
                APP_ERR_RETURN(err, " ** GPT PeriodSet API failed ** \r\n");
                err = R_GPT_Reset (&g_timer0_ctrl);
                APP_ERR_RETURN(err, " ** GPT Reset API failed ** \r\n");
                g_timer_overflow = RESET_FLAG;
                err = R_GPT_Start (&g_timer0_ctrl);
                APP_ERR_RETURN(err, " ** GPT Start API failed ** \r\n");
            }
            /* Reset buffer*/
            read_data = RESET_VALUE;
        }

        return err;
}

/*******************************************************************************************************************//**
 * @brief      User-defined function to draw the current display to a framebuffer.
 *
 * @param[in]  framebuffer    Pointer to frame buffer.
 * @retval     None.
 **********************************************************************************************************************/
static void display_draw (uint32_t * framebuffer)
{
    static uint8_t s_chOffset = 0;
    s_chOffset++;
    /* Draw buffer */
    uint32_t color[COLOR_BAND_COUNT]= {BLUE, LIME, RED, BLACK, WHITE, YELLOW, AQUA, MAGENTA};
    uint16_t bit_width = g_hz_size / COLOR_BAND_COUNT;
    for (uint32_t y = 0; y < g_vr_size; y++)
    {
        for (uint32_t x = 0; x < g_hz_size; x ++)
        {
            uint32_t bit       = x / bit_width;
            framebuffer[x] = color [(bit + s_chOffset) % dimof(color)];
        }
        framebuffer += g_hstride;
    }
}

/*******************************************************************************************************************//**
 * @brief      Touch IRQ callback function
 * NOTE:       This function will return to the highest priority available task.
 * @param[in]  p_args  IRQ callback data
 * @retval     None.
 **********************************************************************************************************************/
void external_irq_callback(external_irq_callback_args_t *p_args)
{
    if (g_external_irq_cfg.channel == p_args->channel)
    {
        g_irq_state =true;
    }
}

/*****************************************************************************************************************
 *  @brief      Process input string to integer value
 *
 *  @param[in]  None
 *  @retval     integer value of input string.
 ****************************************************************************************************************/
static uint8_t process_input_data(void)
{
    unsigned char buf[BUFFER_SIZE_DOWN] = {INITIAL_VALUE};
    uint32_t num_bytes                  = RESET_VALUE;
    uint8_t  value                      = RESET_VALUE;

    while (RESET_VALUE == num_bytes)
    {
        if (APP_CHECK_DATA)
        {
            num_bytes = APP_READ(buf);
            if (RESET_VALUE == num_bytes)
            {
                APP_PRINT("\r\nInvalid Input\r\n");
            }
        }
    }

    /* Conversion from input string to integer value */
    value =  (uint8_t) (atoi((char *)buf));
    return value;
}

/*******************************************************************************************************************//**
 * @brief      This function is used to enter Ultra-low Power State (ULPS) and turn off the backlight.
 *
 * @param[in]  none
 * @retval     none
 **********************************************************************************************************************/
static void mipi_dsi_ulps_enter(void)
{
    fsp_err_t err = FSP_SUCCESS;
    uint32_t timedelay_ms = ENTER_ULPS_DELAY;
    /* Enter Ultra-low Power State (ULPS) */
    g_phy_status = MIPI_DSI_PHY_STATUS_NONE;
    err = R_MIPI_DSI_UlpsEnter (&g_mipi_dsi0_ctrl, (mipi_dsi_lane_t) MIPI_DSI_LANE_DATA_ALL);
    handle_error (err, "** MIPI DSI UlpsEnter API failed ** \r\n");

    /* Wait for a ULPS event */
    err = wait_for_mipi_dsi_event(MIPI_DSI_PHY_STATUS_DATA_LANE_ULPS_ENTER);
    handle_error (err, "** MIPI DSI phy event tỉmeout ** \r\n");
    g_ulps_flag = SET_FLAG;
    APP_PRINT("\r\nEntered Ultra-low Power State (ULPS)\r\n");

    /* Wait about 8 seconds */
    while (!g_irq_state)
    {
        timedelay_ms--;
        R_BSP_SoftwareDelay (1U, BSP_DELAY_UNITS_MICROSECONDS);

        /* Check for time elapse*/
        if (RESET_VALUE == timedelay_ms)
        {
            /* Display Off */
            R_IOPORT_PinWrite (&g_ioport_ctrl, PIN_DISPLAY_BACKLIGHT, BSP_IO_LEVEL_LOW);
            break;
        }
    }
}

/*******************************************************************************************************************//**
 * @brief      This function is used to exit Ultra-low Power State (ULPS) and turn on the backlight.
 *
 * @param[in]  none
 * @retval     none
 **********************************************************************************************************************/
static void mipi_dsi_ulps_exit(void)
{
    fsp_err_t err = FSP_SUCCESS;
    /* Exit Ultra-low Power State (ULPS) */
    g_phy_status = MIPI_DSI_PHY_STATUS_NONE;
    err = R_MIPI_DSI_UlpsExit (&g_mipi_dsi0_ctrl, (mipi_dsi_lane_t) (MIPI_DSI_LANE_DATA_ALL));
    handle_error (err, "** MIPI DSI UlpsExit API failed ** \r\n");

    /* Wait for a ULPS event */
    err = wait_for_mipi_dsi_event(MIPI_DSI_PHY_STATUS_DATA_LANE_ULPS_EXIT);
    handle_error (err, "** MIPI DSI phy event tỉmeout ** \r\n");
    g_ulps_flag = RESET_FLAG;
    APP_PRINT("\r\nExited Ultra-low Power State (ULPS)\r\n");

    /* Display On */
    R_IOPORT_PinWrite (&g_ioport_ctrl, PIN_DISPLAY_BACKLIGHT, BSP_IO_LEVEL_HIGH);
}

/*******************************************************************************************************************//**
 * @brief      This function is used to reset the LCD after power on.
 *
 * @param[in]  none
 * @retval     none
 **********************************************************************************************************************/
void touch_screen_reset(void)
{
     /* Reset touch chip by setting GPIO reset pin low. */
     R_IOPORT_PinWrite(&g_ioport_ctrl, PIN_DISPLAY_RST, BSP_IO_LEVEL_LOW);
     R_IOPORT_PinCfg(&g_ioport_ctrl, PIN_DISPLAY_INT, IOPORT_CFG_PORT_DIRECTION_OUTPUT | IOPORT_CFG_PORT_OUTPUT_LOW);
     R_BSP_SoftwareDelay(200, BSP_DELAY_UNITS_MICROSECONDS);

     /* Start Delay to set the device slave address to 0x28/0x29 */
     R_IOPORT_PinWrite(&g_ioport_ctrl, PIN_DISPLAY_INT, BSP_IO_LEVEL_HIGH);
     R_BSP_SoftwareDelay(200, BSP_DELAY_UNITS_MICROSECONDS);

     /* Release touch chip from reset */
     R_IOPORT_PinWrite(&g_ioport_ctrl, PIN_DISPLAY_RST, BSP_IO_LEVEL_HIGH);
     R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MILLISECONDS);

     /* Set GPIO INT pin low */
     R_IOPORT_PinWrite(&g_ioport_ctrl, PIN_DISPLAY_INT, BSP_IO_LEVEL_LOW);
     R_BSP_SoftwareDelay(100, BSP_DELAY_UNITS_MILLISECONDS);

      /* Release Touch chip interrupt pin for control  */
     R_IOPORT_PinCfg(&g_ioport_ctrl, PIN_DISPLAY_INT, IOPORT_CFG_PORT_DIRECTION_INPUT | IOPORT_CFG_EVENT_RISING_EDGE | IOPORT_CFG_IRQ_ENABLE);

}

/*******************************************************************************************************************//**
 * @brief       This function is used to Wait for mipi dsi event.
 *
 * @param[in]   event   : Events to look forward to
 * @retval      FSP_SUCCESS : Upon successful operation, otherwise: failed
 **********************************************************************************************************************/
static fsp_err_t wait_for_mipi_dsi_event (mipi_dsi_phy_status_t event)
{
    uint32_t timeout = R_FSP_SystemClockHzGet(FSP_PRIV_CLOCK_ICLK) / 10;
    while (timeout-- && ((g_phy_status & event) != event))
    {
        ;
    }
    return timeout ? FSP_SUCCESS : FSP_ERR_TIMEOUT;
}
/*******************************************************************************************************************//**
 *  @brief       This function handles errors, closes all opened modules, and prints errors.
 *
 *  @param[in]   err       error status
 *  @param[in]   err_str   error string
 *  @retval      None
 **********************************************************************************************************************/
void handle_error (fsp_err_t err,  const char * err_str)
{
    if(FSP_SUCCESS != err)
    {
        /* Print the error */
        APP_ERR_PRINT(err_str);

        /* Close opened MIPI_DSI module*/
        if(RESET_VALUE != g_mipi_dsi0_ctrl.open)
        {
            if(FSP_SUCCESS != R_MIPI_DSI_Close (&g_mipi_dsi0_ctrl))
            {
                APP_ERR_PRINT("MIPI DSI Close API failed\r\n");
            }
        }

        /* Close opened GPT module*/
        if(RESET_VALUE != g_timer0_ctrl.open)
        {
            if(FSP_SUCCESS != R_GPT_Close (&g_timer0_ctrl))
            {
                APP_ERR_PRINT("GPT Close API failed\r\n");
            }
        }

        /* Close opened GLCD module*/
        if(RESET_VALUE != g_display_ctrl.state)
        {
            if(FSP_SUCCESS != R_GLCDC_Close (&g_display_ctrl))
            {
                APP_ERR_PRINT("GLCDC Close API failed\r\n");
            }
        }

        /* Close opened ICU module*/
        if(RESET_VALUE != g_external_irq_ctrl.open)
        {
            if(FSP_SUCCESS != R_ICU_ExternalIrqClose (&g_external_irq_ctrl))
            {
                APP_ERR_PRINT("ICU ExternalIrqClose API failed\r\n");
            }
        }

        /* Close opened IIC master module*/
        if(RESET_VALUE != g_i2c_master_ctrl.open)
        {
            if(FSP_SUCCESS != R_IIC_MASTER_Close(&g_i2c_master_ctrl))
            {
                APP_ERR_PRINT("IIC MASTER Close API failed\r\n");
            }
        }

        APP_ERR_TRAP(err);
    }
}

/*******************************************************************************************************************//**
 * @brief      Callback functions for GLCDC interrupts
 *
 * @param[in]  p_args    Callback arguments
 * @retval     none
 **********************************************************************************************************************/
void glcdc_callback (display_callback_args_t * p_args)
{
    if (DISPLAY_EVENT_LINE_DETECTION == p_args->event)
    {
        g_vsync_flag = SET_FLAG;
#if __DISP0_CFG_ENABLE_3FB_HELPER_SERVICE__
        disp_adapter0_3fb_get_flush_pointer();
#endif
    }
}

/*******************************************************************************************************************//**
 * @brief      Callback functions for MIPI DSI interrupts
 *
 * @param[in]  p_args    Callback arguments
 * @retval     none
 **********************************************************************************************************************/
void mipi_dsi_callback(mipi_dsi_callback_args_t *p_args)
{
    switch (p_args->event)
    {
        case MIPI_DSI_EVENT_SEQUENCE_0:
        {
            if (MIPI_DSI_SEQUENCE_STATUS_DESCRIPTORS_FINISHED == p_args->tx_status)
            {
                g_message_sent = SET_FLAG;
            }
            break;
        }
        case MIPI_DSI_EVENT_PHY:
        {
            g_phy_status |= p_args->phy_status;
            break;
        }
        default:
        {
            break;
        }

    }
}

/*******************************************************************************************************************//**
 * @brief      Callback functions for gpt interrupts
 *
 * @param[in]  p_args    Callback arguments
 * @retval     none
 **********************************************************************************************************************/
void gpt_callback(timer_callback_args_t *p_args)
{
    /* Check for the event */
    if (TIMER_EVENT_CYCLE_END == p_args->event)
    {
        g_timer_overflow = SET_FLAG;
    }
}
/*******************************************************************************************************************//**
 * @} (end addtogroup mipi_dsi_ep)
 **********************************************************************************************************************/
