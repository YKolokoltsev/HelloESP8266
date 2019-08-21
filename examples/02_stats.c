/**
 * Send chip statistics over UART0 each 2 seconds.
 */

#include <user_interface.h>
#include <osapi.h>
#include <uart.h>

static os_timer_t timer;

typedef unsigned int uint;

/**
 * In this example we send some chip information over UART interface.
 * This is a basic example that shell work, because this way we would
 * have a feedback from the chip during another examples.
 *
 * To be sure that flashing process over UART0 will not cross with
 * system serial monitor we send the same statistics each 2 seconds
 * using timer interrupt.
 */

void
timer_interrupt() {

    uint32 heap_size = system_get_free_heap_size();

    // Get MAC addresses.
    uint8 soft_ap_mac[6];
    uint8 station_mac[6];
    wifi_get_macaddr(SOFTAP_IF, soft_ap_mac);
    wifi_get_macaddr(STATION_IF, station_mac);

    os_printf("\n\n");

    os_printf("Hello Universe!\n");

    os_printf("\n\n");

    // System info.
    os_printf("Chip ID        : %d\n", system_get_chip_id());
    os_printf("CPU freq       : %d\n", system_get_cpu_freq());
    os_printf("Free heap size : %d\n", heap_size);
    os_printf("SDK version    : %s\n", system_get_sdk_version());
    os_printf("Soft AP MAC    : "MACSTR"\n", MAC2STR(soft_ap_mac));
    os_printf("Station MAC    : "MACSTR"\n", MAC2STR(station_mac));

    os_printf("\n");

    system_print_meminfo();

    os_printf("\n");

    // Type sizes.
    os_printf("Type sizes in bytes.\n");
    os_printf("  char    : %x byte\n",    (uint) sizeof(char));
    os_printf("  short   : %x bytes\n",   (uint) sizeof(short));
    os_printf("  int     : %x bytes\n",   (uint) sizeof(int));
    os_printf("  long    : %x bytes\n",   (uint) sizeof(long));
    os_printf("  size_t  : %x bytes\n",   (uint) sizeof(size_t));
    os_printf("  float   : %x bytes\n",   (uint) sizeof(float));
    os_printf("  double  : %x bytes\n\n", (uint) sizeof(double));
    os_printf("  pointer : %x bytes\n",   (uint) sizeof(int *));

    os_printf("\n\n");

    // MSB or LSB first.
    uint16_t i = 0xABCD;
    uint8_t *p = (uint8_t *) &i;

    if (*p == 0xAB) {
        os_printf("System is MSB first:\n");
    } else {
        os_printf("System is LSB first:\n");
    }

    os_printf("*p     : %X\n", *p);
    os_printf("*(p+1) : %X\n", *(p+1));
}


/**
 * The entry point to your program.
 *
 * Sets up UART and schedules call to user code.
 */
void ICACHE_FLASH_ATTR
user_init() {

    // Initialize UART0.
    uart_div_modify(0, UART_CLK_FREQ / BIT_RATE_74880);

    // No need for wifi for this example.
    wifi_station_disconnect();
    wifi_set_opmode_current(NULL_MODE);

    // setup timer (2000ms, repeating)
    os_timer_setfn(&timer, (os_timer_func_t *)timer_interrupt, NULL);
    os_timer_arm(&timer, 2000, 1);
}