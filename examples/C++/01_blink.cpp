/**
 * It is well known that software emulation is bad. Let's see how it works.... (compare software timer with hw timer)
 */


/*
 * ESP8266-NOOS-SDK is a pure C library and does not extern any C function declaration
 * for that it can be used in a C++ code. Therefore, as a rule, all SDK includes has to be enclosed within
 * extern "C" { ... } block. In the opposite case the C++ linker will not find any of the library functions.
 *
 * This is because C++ compiler permits to use the same function in a different namespaces and after compilation
 * assigns to each function a different unique name (name-mangling). extern "C" tells the C++ compiler not
 * to perform any name-mangling on the code within the braces. This allows you to call C functions from within C++.
 */

extern "C" {

    //Software timer does not support us precision. Following documentation it shell have
    //500us jitter between it's callback function calls. 
    #define USE_US_TIMER

    #include <osapi.h>
    #include <gpio.h>
}

static const int LED_GPIO = 2;
static volatile os_timer_t timer;

void
timer_interrupt(void *arg)
{
    if (GPIO_REG_READ(GPIO_OUT_ADDRESS) & (1 << LED_GPIO))
    {
        gpio_output_set(0, (1 << LED_GPIO), 0, 0);
    }
    else
    {
        gpio_output_set((1 << LED_GPIO), 0, 0, 0);
    }
}

// Same reason for extern "C" here because. 'user_init' function is referenced from C binary libraries of SDK
// and it will not be found without 'extern' directive.
extern "C" {
void ICACHE_FLASH_ATTR
user_init() {
    gpio_init();

    PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_GPIO1);
    gpio_output_set(0, 0, (1 << LED_GPIO), 0);

    os_timer_setfn((os_timer_t *) &timer, (os_timer_func_t *) timer_interrupt, NULL);
    system_timer_reinit();
    
    //theoretically shell be 1us timer, but it is not
    os_timer_arm_us((os_timer_t * ) & timer, 1, 1);
}
}

/*
 * TODO:
 * 1. Connect your oscilloscope to the pin and measure frequency and it's stability
 * 2. Configure hw timer and do the same
 * 3. When we use software timer and we we use hw timer?
 */