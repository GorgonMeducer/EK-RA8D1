/* generated vector source file - do not edit */
        #include "bsp_api.h"
        /* Do not build these data structures if no interrupts are currently allocated because IAR will have build errors. */
        #if VECTOR_DATA_IRQ_COUNT > 0
        BSP_DONT_REMOVE const fsp_vector_t g_vector_table[BSP_ICU_VECTOR_MAX_ENTRIES] BSP_PLACE_IN_SECTION(BSP_SECTION_APPLICATION_VECTORS) =
        {
                        [0] = glcdc_line_detect_isr, /* GLCDC LINE DETECT (Specified line) */
            [1] = mipi_dsi_seq0, /* DSI SEQ0 */
            [2] = mipi_dsi_seq1, /* DSI SEQ1 */
            [3] = mipi_dsi_vin1, /* DSI VIN1 */
            [4] = mipi_dsi_rcv, /* DSI RCV */
            [5] = mipi_dsi_ferr, /* DSI FERR */
            [6] = mipi_dsi_ppi, /* DSI PPI */
            [7] = r_icu_isr, /* ICU IRQ3 (External pin interrupt 3) */
            [8] = iic_master_rxi_isr, /* IIC1 RXI (Receive data full) */
            [9] = iic_master_txi_isr, /* IIC1 TXI (Transmit data empty) */
            [10] = iic_master_tei_isr, /* IIC1 TEI (Transmit end) */
            [11] = iic_master_eri_isr, /* IIC1 ERI (Transfer error) */
            [12] = gpt_counter_overflow_isr, /* GPT0 COUNTER OVERFLOW (Overflow) */
        };
        const bsp_interrupt_event_t g_interrupt_event_link_select[BSP_ICU_VECTOR_MAX_ENTRIES] =
        {
            [0] = BSP_PRV_IELS_ENUM(EVENT_GLCDC_LINE_DETECT), /* GLCDC LINE DETECT (Specified line) */
            [1] = BSP_PRV_IELS_ENUM(EVENT_MIPI_DSI_SEQ0), /* DSI SEQ0 */
            [2] = BSP_PRV_IELS_ENUM(EVENT_MIPI_DSI_SEQ1), /* DSI SEQ1 */
            [3] = BSP_PRV_IELS_ENUM(EVENT_MIPI_DSI_VIN1), /* DSI VIN1 */
            [4] = BSP_PRV_IELS_ENUM(EVENT_MIPI_DSI_RCV), /* DSI RCV */
            [5] = BSP_PRV_IELS_ENUM(EVENT_MIPI_DSI_FERR), /* DSI FERR */
            [6] = BSP_PRV_IELS_ENUM(EVENT_MIPI_DSI_PPI), /* DSI PPI */
            [7] = BSP_PRV_IELS_ENUM(EVENT_ICU_IRQ3), /* ICU IRQ3 (External pin interrupt 3) */
            [8] = BSP_PRV_IELS_ENUM(EVENT_IIC1_RXI), /* IIC1 RXI (Receive data full) */
            [9] = BSP_PRV_IELS_ENUM(EVENT_IIC1_TXI), /* IIC1 TXI (Transmit data empty) */
            [10] = BSP_PRV_IELS_ENUM(EVENT_IIC1_TEI), /* IIC1 TEI (Transmit end) */
            [11] = BSP_PRV_IELS_ENUM(EVENT_IIC1_ERI), /* IIC1 ERI (Transfer error) */
            [12] = BSP_PRV_IELS_ENUM(EVENT_GPT0_COUNTER_OVERFLOW), /* GPT0 COUNTER OVERFLOW (Overflow) */
        };
        #endif