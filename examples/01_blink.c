/**
 * Blink diode on the GPIO2 pin (correct for ESP-12 or ESP-12F boards)
 */

#include <osapi.h>
#include <gpio.h>

/**
 * ESP-12 modules have LED on GPIO2. Change to another GPIO
 * for other boards.
 */
static const int LED_GPIO = 2;
static os_timer_t timer; //volatile

void
timer_interrupt(void *arg)
{
    //Do blinky stuff
    if (GPIO_REG_READ(GPIO_OUT_ADDRESS) & (1 << LED_GPIO))
    {
        // set gpio low
        gpio_output_set(0, (1 << LED_GPIO), 0, 0);
    }
    else
    {
        // set gpio high
        gpio_output_set((1 << LED_GPIO), 0, 0, 0);
    }
}

void ICACHE_FLASH_ATTR
user_init()
{
    // init gpio subsytem
    gpio_init();

    // configure UART TXD to be GPIO1, set as output
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_GPIO1);
    gpio_output_set(0, 0, (1 << LED_GPIO), 0);

    // setup timer (500ms, repeating)
    os_timer_setfn(&timer, (os_timer_func_t *)timer_interrupt, NULL);
    os_timer_arm(&timer, 500, 1);
}