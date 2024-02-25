/**
 * @file lv_port_disp_templ.c
 *
 */

/*Copy this file as "lv_port_disp.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_disp_template.h"
#include <stdbool.h>

#if defined(_RTE_)
#   include "RTE_Components.h"

#   if defined(__RTE_ACCELERATION_ARM_2D__)
#       include "arm_2d.h"
#       include "__arm_2d_impl.h"
#   endif
#endif

/*********************
 *      DEFINES
 *********************/
#ifndef MY_DISP_HOR_RES
    #define MY_DISP_HOR_RES    480
#endif

#ifndef MY_DISP_VER_RES
    #define MY_DISP_VER_RES    854
#endif

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void disp_init(void);

static void disp_flush(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
extern void Disp0_DrawBitmap (uint32_t x, uint32_t y, uint32_t width, uint32_t height, const uint8_t *bitmap);


void lv_port_disp_init(void)
{
    /*-------------------------
     * Initialize your display
     * -----------------------*/
    disp_init();

    /*------------------------------------
     * Create a display and set a flush_cb
     * -----------------------------------*/
    lv_display_t * disp = lv_display_create(MY_DISP_HOR_RES, MY_DISP_VER_RES);
    lv_display_set_flush_cb(disp, disp_flush);

    /* Example 1
     * One buffer for partial rendering*/
    static lv_color_t buf_1_1[MY_DISP_HOR_RES * MY_DISP_VER_RES / 10];          /*A buffer for 1/10 FB*/
    lv_display_set_buffers(disp, buf_1_1, NULL, sizeof(buf_1_1), LV_DISPLAY_RENDER_MODE_PARTIAL);

}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*Initialize your display and the required peripherals.*/
static void disp_init(void)
{
    /*You code here*/
}

volatile bool disp_flush_enabled = true;

/* Enable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_enable_update(void)
{
    disp_flush_enabled = true;
}

/* Disable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_disable_update(void)
{
    disp_flush_enabled = false;
}

/*Flush the content of the internal buffer the specific area on the display.
 *`px_map` contains the rendered image as raw pixel map and it should be copied to `area` on the display.
 *You can use DMA or any hardware acceleration to do this operation in the background but
 *'lv_display_flush_ready()' has to be called when it's finished.*/
static void disp_flush(lv_display_t * disp_drv, const lv_area_t * area, uint8_t * px_map)
{

    if (disp_flush_enabled) {

        int32_t stride = 0, width = 0, height = 0;
        do {
            lv_color_format_t color_format = lv_display_get_color_format(disp_drv);
            width = area->x2 - area->x1 + 1;
            height = area->y2 - area->y1 + 1;
            stride = lv_draw_buf_width_to_stride( width, color_format);
            int32_t stride_from_width = width * (lv_color_format_get_bpp(color_format) >> 3);
            
            if (stride == stride_from_width) {
                break;
            }
            
            uint8_t *src_ptr = px_map;
            uint8_t *des_ptr = px_map;
             
            for (int y = 0; y < height; y++) {
                lv_memcpy(des_ptr, src_ptr, stride_from_width);
                
                src_ptr += stride;
                des_ptr += stride_from_width;
            }
        } while(0);

    #if defined(__RTE_ACCELERATION_ARM_2D__)
        if (LV_COLOR_DEPTH == 16) {
            /* we need a color format conversion */
            static uint32_t buf_32[MY_DISP_HOR_RES * MY_DISP_VER_RES / 5];
            
            __arm_2d_impl_rgb565_to_cccn888(
                (uint16_t *)px_map,
                width,//stride / sizeof(uint16_t),
                buf_32,
                width,
                (arm_2d_size_t []){{
                    width, height
                }});
            
            px_map = (uint8_t *)buf_32;
        }
    #endif

        Disp0_DrawBitmap(area->x1,               //!< x
                         area->y1,               //!< y
                         width,    //!< width
                         height,    //!< height
                         (const uint8_t *)px_map);

    }


    /*IMPORTANT!!!
     *Inform the graphics library that you are ready with the flushing*/
    lv_display_flush_ready(disp_drv);
}

#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
