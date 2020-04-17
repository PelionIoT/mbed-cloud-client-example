/* generated thread source file - do not edit */
#include "net_thread.h"

#if 1
static StaticTask_t net_thread_memory;
static uint8_t net_thread_stack[4096] BSP_PLACE_IN_SECTION(".stack.net_thread") BSP_ALIGN_VARIABLE(BSP_STACK_ALIGNMENT);
#endif
TaskHandle_t net_thread;
void net_thread_create(void);
static void net_thread_func(void *pvParameters);
void rtos_startup_err_callback(void *p_instance, void *p_data);
void rtos_startup_common_init(void);
extern uint32_t g_fsp_common_thread_count;

const rm_freertos_port_parameters_t net_thread_parameters =
{ .p_context = (void *) NULL, };

void net_thread_create(void)
{
    /* Increment count so we will know the number of ISDE created threads. */
    g_fsp_common_thread_count++;

    /* Initialize each kernel object. */

#if 1
    net_thread = xTaskCreateStatic (
#else
                                    BaseType_t net_thread_create_err = xTaskCreate(
#endif
                                    net_thread_func,
                                    (const char *) "Net Thread", 4096 / 4, // In words, not bytes
                                    (void *) &net_thread_parameters, //pvParameters
                                    2,
#if 1
                                    (StackType_t *) &net_thread_stack,
                                    (StaticTask_t *) &net_thread_memory
#else
                                    & net_thread
#endif
                                    );

#if 1
    if (NULL == net_thread)
    {
        rtos_startup_err_callback (net_thread, 0);
    }
#else
    if (pdPASS != net_thread_create_err)
    {
        rtos_startup_err_callback(net_thread, 0);
    }
#endif
}
static void net_thread_func(void *pvParameters)
{
    /* Initialize common components */
    rtos_startup_common_init ();

    /* Initialize each module instance. */

    /* Enter user code for this thread. Pass task handle. */
    net_thread_entry (pvParameters);
}
