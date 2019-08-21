#include <osapi.h>
#include <user_interface.h>
#include <smartconfig.h>

#define SSID "INFINITUMA8F2"
#define PASSWORD "1837497823"

static os_timer_t timer; //volatile
struct station_config config;

void
user_timer_interrupt(void *arg)
{
    os_printf("Wi-Fi operationg mode: ");
    switch(wifi_get_opmode()){
        case(NULL_MODE): os_printf("%s\n", "NULL_MODE"); break;
        case(STATION_MODE): os_printf("%s\n", "STATION_MODE"); break;
        case(SOFTAP_MODE): os_printf("%s\n", "SOFTAP_MODE"); break;
        case(STATIONAP_MODE): os_printf("%s\n", "STATIONAP_MODE"); break;
        default: os_printf("%s\n", "UNDEFINED"); break;
    }

    wifi_station_get_config(&config);

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
    memset(hostname, 0, sizeof(hostname));
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

    memset(&config, 0, sizeof(struct station_config));
    memcpy(config.ssid, SSID, sizeof(SSID)-1);
    memcpy(config.password, PASSWORD, sizeof(PASSWORD)-1);

    wifi_station_set_config_current(&config);
    wifi_station_set_reconnect_policy(true);

    // Setup timer (2000ms, repeating)
    os_timer_setfn(&timer, (os_timer_func_t *)user_timer_interrupt, NULL);
    os_timer_arm(&timer, 2000, 1);
}