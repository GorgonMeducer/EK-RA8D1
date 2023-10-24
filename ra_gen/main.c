/* generated main source file - do not edit */
#include "hal_data.h"
#include "RTE_Components.h"
#include "cmsis_compiler.h"
#include "perf_counter.h"




#if defined(RTE_CMSIS_RTOS2)

#define APP_STACK_SIZE   4096

#include "cmsis_os2.h"

 __NO_RETURN
void app_2d_main_thread (void *argument) 
{
    hal_entry();

    osThreadExit();
}


int main(void) 
{
    /* Initialize CMSIS-RTOS2 */
    osKernelInitialize ();


    static uint64_t thread1_stk_1[APP_STACK_SIZE / sizeof(uint64_t)];
     
    const osThreadAttr_t thread1_attr = {
      .stack_mem  = &thread1_stk_1[0],
      .stack_size = sizeof(thread1_stk_1)
    };

    /* Create application main thread */
    osThreadNew(app_2d_main_thread, NULL, &thread1_attr);

    /* Start thread execution */
    osKernelStart();

    while (1) {
    }

    return 0;
}
#else
int main(void) 
{
    hal_entry();
    return 0;
}
#endif
