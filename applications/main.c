

#include <rtthread.h>
#include <rtdevice.h>
#include "BYH_M.h"
#include "NB_IOT.h"

/* defined the LED0 pin: PB5 */
#define LED0_PIN    135
/* defined the LED0 pin: PE5 */
#define LED1_PIN    4

int main(void)
{
    int count = 1;
	
    /* set LED0 pin mode to output */
    rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);
    /* set LED1 pin mode to output */
//    rt_pin_mode(LED1_PIN, PIN_MODE_OUTPUT);

    while (count++)
    {
        rt_pin_write(LED0_PIN, PIN_HIGH);
//        rt_pin_write(LED1_PIN, PIN_HIGH);
        rt_thread_mdelay(500);
        rt_pin_write(LED0_PIN, PIN_LOW);
//        rt_pin_write(LED1_PIN, PIN_LOW);
        rt_thread_mdelay(500);
    }

    return RT_EOK;
}

