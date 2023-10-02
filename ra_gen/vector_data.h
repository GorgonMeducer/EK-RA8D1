/* generated vector header file - do not edit */
        #ifndef VECTOR_DATA_H
        #define VECTOR_DATA_H
        #ifdef __cplusplus
        extern "C" {
        #endif
                /* Number of interrupts allocated */
        #ifndef VECTOR_DATA_IRQ_COUNT
        #define VECTOR_DATA_IRQ_COUNT    (13)
        #endif
        /* ISR prototypes */
        void glcdc_line_detect_isr(void);
        void mipi_dsi_seq0(void);
        void mipi_dsi_seq1(void);
        void mipi_dsi_vin1(void);
        void mipi_dsi_rcv(void);
        void mipi_dsi_ferr(void);
        void mipi_dsi_ppi(void);
        void r_icu_isr(void);
        void iic_master_rxi_isr(void);
        void iic_master_txi_isr(void);
        void iic_master_tei_isr(void);
        void iic_master_eri_isr(void);
        void gpt_counter_overflow_isr(void);

        /* Vector table allocations */
        #define VECTOR_NUMBER_GLCDC_LINE_DETECT ((IRQn_Type) 0) /* GLCDC LINE DETECT (Specified line) */
        #define GLCDC_LINE_DETECT_IRQn          ((IRQn_Type) 0) /* GLCDC LINE DETECT (Specified line) */
        #define VECTOR_NUMBER_MIPI_DSI_SEQ0 ((IRQn_Type) 1) /* DSI SEQ0 */
        #define MIPI_DSI_SEQ0_IRQn          ((IRQn_Type) 1) /* DSI SEQ0 */
        #define VECTOR_NUMBER_MIPI_DSI_SEQ1 ((IRQn_Type) 2) /* DSI SEQ1 */
        #define MIPI_DSI_SEQ1_IRQn          ((IRQn_Type) 2) /* DSI SEQ1 */
        #define VECTOR_NUMBER_MIPI_DSI_VIN1 ((IRQn_Type) 3) /* DSI VIN1 */
        #define MIPI_DSI_VIN1_IRQn          ((IRQn_Type) 3) /* DSI VIN1 */
        #define VECTOR_NUMBER_MIPI_DSI_RCV ((IRQn_Type) 4) /* DSI RCV */
        #define MIPI_DSI_RCV_IRQn          ((IRQn_Type) 4) /* DSI RCV */
        #define VECTOR_NUMBER_MIPI_DSI_FERR ((IRQn_Type) 5) /* DSI FERR */
        #define MIPI_DSI_FERR_IRQn          ((IRQn_Type) 5) /* DSI FERR */
        #define VECTOR_NUMBER_MIPI_DSI_PPI ((IRQn_Type) 6) /* DSI PPI */
        #define MIPI_DSI_PPI_IRQn          ((IRQn_Type) 6) /* DSI PPI */
        #define VECTOR_NUMBER_ICU_IRQ3 ((IRQn_Type) 7) /* ICU IRQ3 (External pin interrupt 3) */
        #define ICU_IRQ3_IRQn          ((IRQn_Type) 7) /* ICU IRQ3 (External pin interrupt 3) */
        #define VECTOR_NUMBER_IIC1_RXI ((IRQn_Type) 8) /* IIC1 RXI (Receive data full) */
        #define IIC1_RXI_IRQn          ((IRQn_Type) 8) /* IIC1 RXI (Receive data full) */
        #define VECTOR_NUMBER_IIC1_TXI ((IRQn_Type) 9) /* IIC1 TXI (Transmit data empty) */
        #define IIC1_TXI_IRQn          ((IRQn_Type) 9) /* IIC1 TXI (Transmit data empty) */
        #define VECTOR_NUMBER_IIC1_TEI ((IRQn_Type) 10) /* IIC1 TEI (Transmit end) */
        #define IIC1_TEI_IRQn          ((IRQn_Type) 10) /* IIC1 TEI (Transmit end) */
        #define VECTOR_NUMBER_IIC1_ERI ((IRQn_Type) 11) /* IIC1 ERI (Transfer error) */
        #define IIC1_ERI_IRQn          ((IRQn_Type) 11) /* IIC1 ERI (Transfer error) */
        #define VECTOR_NUMBER_GPT0_COUNTER_OVERFLOW ((IRQn_Type) 12) /* GPT0 COUNTER OVERFLOW (Overflow) */
        #define GPT0_COUNTER_OVERFLOW_IRQn          ((IRQn_Type) 12) /* GPT0 COUNTER OVERFLOW (Overflow) */
        #ifdef __cplusplus
        }
        #endif
        #endif /* VECTOR_DATA_H */