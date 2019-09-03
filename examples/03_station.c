/**
 * In this example we try to connect to the Acess Point
 * specifiyed by SSID and PASSWORD constants. Afterwords we send
 * connection stats over UART0 interface periodically.
 */

#define SSID "INFINITUMA8F2"
#define PASSWORD "1837497823"

#include <osapi.h>
#include <user_interface.h>
#include <smartconfig.h>

volatile static os_timer_t timer;

/*
 * The "config" struct is used when we setup connection to the AP,
 * but when connection was accepted by AP and established, this struct gets additional
 * information. The moment when it happens is ruled by internal SDK interrupt,
 * so this structure is volatile. It is static because we do not use it in any other C source file.
 * Also C langauge standard say that struct variable name can cross with the names of another identifiers,
 * by this reason we use a keyword "struct".
 *
 * Note: yes, everything is important with global variables, remember that!
 */

volatile static struct station_config config;

void
timer_interrupt(void *arg)
{
    os_printf("Wi-Fi operationg mode: ");
    switch(wifi_get_opmode()){
        case(NULL_MODE): os_printf("%s\n", "NULL_MODE"); break;
        case(STATION_MODE): os_printf("%s\n", "STATION_MODE"); break;
        case(SOFTAP_MODE): os_printf("%s\n", "SOFTAP_MODE"); break;
        case(STATIONAP_MODE): os_printf("%s\n", "STATIONAP_MODE"); break;
        default: os_printf("%s\n", "UNDEFINED"); break;
    }

    wifi_station_get_config((struct station_config*)&config);

    os_printf("AP SSID: %s\n",SSID);
    os_printf("Password: %s\n",PASSWORD);
    os_printf("Autodetected AP BSSID: %02X:%02X:%02X:%02X:%02X:%02X\n",
            config.bssid[0], config.bssid[1], config.bssid[2],
            config.bssid[3], config.bssid[4], config.bssid[5]);
    os_printf("BSSID check: %s\n",config.bssid_set ? "YES" : "NO");

    os_printf("Connection status: ");
    switch(wifi_station_get_connect_status()){
        case(STATION_IDLE): os_printf("%s\n", "STATION_IDLE"); break;
        case(STATION_CONNECTING): os_printf("%s\n", "STATION_CONNECTING"); break;
        case(STATION_WRONG_PASSWORD): os_printf("%s\n", "STATION_WRONG_PASSWORD"); break;
        case(STATION_NO_AP_FOUND): os_printf("%s\n", "STATION_NO_AP_FOUND"); break;
        case(STATION_CONNECT_FAIL): os_printf("%s\n", "STATION_CONNECT_FAIL"); break;
        case(STATION_GOT_IP): os_printf("%s\n", "STATION_GOT_IP"); break;
        default: os_printf("%s\n", "UNDEFINED"); break;
    }

    struct ip_info info;

    if(wifi_get_ip_info(STATION_IF, &info)){
        os_printf("IP: %d.%d.%d.%d\n", IP2STR(&info.ip));
        os_printf("Netmask: %d.%d.%d.%d\n", IP2STR(&info.netmask));
        os_printf("Gateway: %d.%d.%d.%d\n", IP2STR(&info.gw));
    }else{
        os_printf("Connection info: UNDEFINED\n");
    }

}

void ICACHE_FLASH_ATTR
user_init()
{
    // Setup Wi-Fi in a station-only current mode (do not update flash)
    wifi_set_opmode_current(STATION_MODE);

    // Setup unique station hostname based on it's chip ID
    char hostname[32];
    os_memset(hostname, 0, sizeof(hostname));
    ets_sprintf(hostname, "ESP8266-%u",system_get_chip_id());
    wifi_station_set_hostname(hostname);


    /**
     * Connect to the Access Point
     *
     * If wifi_station_set_config is called in user_init , there is no need to call
     * wifi_station_connect after that, ESP8266 will connect to router
     * automatically; otherwise, wifi_station_connect is needed to connect.
     *
     * While connecting, the config.ssid will be cleaned up and config.bssid (AP MAC)
     * will be set correctly.
     */

    os_memset((struct station_config*) &config, 0, sizeof(struct station_config));

    // 'ssid' and 'password' are pointers! see it's definition in the user_interface.h
    os_memcpy((void*)config.ssid, SSID, sizeof(SSID)-1);
    os_memcpy((void*)config.password, PASSWORD, sizeof(PASSWORD)-1);

    wifi_station_set_config_current((struct station_config*) &config);
    wifi_station_set_reconnect_policy(true);

    // Setup timer (2000ms, repeating)
    os_timer_setfn((os_timer_t*) &timer, (os_timer_func_t *)timer_interrupt, NULL);
    os_timer_arm((os_timer_t*) &timer, 2000, 1);
}

/*
 * TODO:
 * 1. Change SSID and password constants into your AP.
 *
 * 2. Implement the wifi_event_cb(System_Event_t *event) interrupt function, reed
 * the http://smallbits.marshall-tribe.net/blog/2016/05/21/esp8266-networking-basics article
 * for the references.
 *
 * 3. Report different events over UART and measure when they happen. To get more error events
 * modify SSID and password to be incorrect.
 *
 * Report the results and discuss them.
 */