/**
 * Blink diode on the GPIO2 pin (correct for ESP-12 or ESP-12F boards)
 */

#include <osapi.h>
#include <gpio.h>

/**
 * ESP-12 modules have LED on GPIO2. Change to another GPIO
 * for other boards.
 */

 // static <-> global variable is used only within this *.c file
 // const <-> tell to compiler that it can use the value of the variable explicitly without referencing to it
static const int LED_GPIO = 2;

// volatile <-> the value of this variable can change outside of the code
// (some processes in microcontrollers pass in parallel on a hardware level)
static volatile os_timer_t timer;

// this function will be called by microcontroller when it's internal timer
// reaches a value defined below (see user_init)
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

/**
 * This is an entry point function, it is called just once by microcontroller to initialize it's subsystems.
 * Afterwards all processing is based on interrupts.
 *
 * In ESP8266 exist an optimization that permits to release
 * RAM from those functions that are not needed permanently, such as 'user_init()'. To achieve this
 * you should decorate such function with the "ICACHE_FLASH_ATTR" macro. In this case such function will
 * be loaded into RAM only when required.
 */
void ICACHE_FLASH_ATTR
user_init()
{
    // init gpio subsytem
    gpio_init();

    // configure UART TXD to be GPIO1, set as output
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_GPIO1);
    gpio_output_set(0, 0, (1 << LED_GPIO), 0);

    // setup timer (500ms, repeating)
    os_timer_setfn((os_timer_t*) &timer, (os_timer_func_t *)timer_interrupt, NULL);
    os_timer_arm((os_timer_t*) &timer, 500, 1);
}

/*
 * TODO:
 * 1. Describe hoe bitwise operators work
 *
 * 2. Wat means "mask" in the "gpio_output_set" function? (see Non_OS_SDK_Reference)
 *
 * 3. Read an article on volatile variables and explain why "timer" is better to
 * declare as volatile:
 * https://barrgroup.com/Embedded-Systems/How-To/C-Volatile-Keyword
 */