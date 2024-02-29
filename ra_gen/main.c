/* generated main source file - do not edit */
#include "hal_data.h"
#include "RTE_Components.h"
#include "cmsis_compiler.h"
#include "perf_counter.h"



#if defined(RTE_CMSIS_RTOS2)

#define APP_STACK_SIZE   4096

#include "cmsis_os2.h"

#if defined(RTE_CMSIS_RTOS2_FreeRTOS)
#   include "FreeRTOS.h"
#endif

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

#if defined(RTE_CMSIS_RTOS2_FreeRTOS)
    static StaticTask_t s_tTaskCB = {0};
#endif
    static uint64_t thread1_stk_1[APP_STACK_SIZE / sizeof(uint64_t)];

    const osThreadAttr_t thread1_attr = {
#if defined(RTE_CMSIS_RTOS2_FreeRTOS)
      .cb_mem = &s_tTaskCB,
      .cb_size = sizeof(s_tTaskCB),
#endif
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

 #if __IS_COMPILER_ARM_COMPILER_6__
__asm(".global __use_no_semihosting\n\t");
#ifndef __MICROLIB
__asm(".global __ARM_use_no_argv\n\t");
#endif

void _sys_exit(int ch)
{
    (void)ch;
    while(1);
}


void _ttywrch(int ch)
{
    (void)ch;
}

//#include <rt_sys.h>

//FILEHANDLE $Sub$$_sys_open(const char *name, int openmode)
//{
//    (void)name;
//    (void)openmode;
//    return 0;
//}

#endif