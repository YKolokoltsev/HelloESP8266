#include <osapi.h>
#include <user_interface.h>
#include <espconn.h>

//wifi_register_send_pkt_freedom_cb

//TODO: Read this!
//http://smallbits.marshall-tribe.net/blog/2016/05/21/esp8266-networking-basics

#define SSID "INFINITUMA8F2"
#define PASSWORD "1837497823"
const uint8 r_ip_a = 192, r_ip_b = 168, r_ip_c = 1, r_ip_d = 64;
const uint16 r_port = 1234;
uint8 DATA_BUF[] = "ABCDE_";

// ******************************************

static os_timer_t timer; //volatile
struct espconn conn;

/**
 * Callback after the data are sent.
 *
 * @param arg pointer corresponding structure espconn
 */
void
sent_callback(void *arg){
    os_printf("sent_callback: Ok\n");
}

/**
 * The timer works as an UDP transmission watchdog
 * @param arg
 */
void
timer_interrupt(void *arg) {

    // Reconnection to AP is made automatically, if
    // something is wrong try to log output uf the 03_station over UART0.
    if(wifi_station_get_connect_status() != STATION_GOT_IP)
        return;

    remot_info *premot = NULL;

    if(espconn_get_connection_info( &conn, &premot, 0) != ESPCONN_OK){
        // Register connection to remote UDP port
        conn.proto.udp = &udp_proto;
        bool res;

        switch(espconn_create(&conn)){
            case(ESPCONN_ISCONN): os_printf("espconn_create: Already connected\n"); res = false; break;
            case(ESPCONN_MEM): os_printf("espconn_create: Out of memory\n"); res = false; break;
            case(ESPCONN_ARG): os_printf("espconn_create: cannot find UDP transmission according to espconn\n"); res = false; break;
            case(0): os_printf("espconn_create: Ok\n"); res = true; break;
            default: os_printf("espconn_create: UNDEFINED\n"); res = false; break;
        };

        if(res){
            switch(espconn_regist_sentcb(&conn, sent_callback)){
                case(ESPCONN_ARG): os_printf("espconn_regist_sentcb: cannot find UDP transmission according to espconn\n"); break;
                case(0): os_printf("espconn_regist_sentcb: Ok\n"); break;
                default: os_printf("espconn_regist_sentcb: UNDEFINED\n"); res = false; break;
            }
        }

    }else{
        // Send some data
        switch(espconn_send(&conn, DATA_BUF, sizeof(DATA_BUF))){
            case(ESPCONN_MEM): os_printf("espconn_send: Out of memory\n"); break;
            case(ESPCONN_ARG): os_printf("espconn_send: Cannot find network transmission\n"); break;
            case(ESPCONN_MAXNUM): os_printf("espconn_send: Sending buffer is full\n"); break;
            case(ESPCONN_IF): os_printf("espconn_send: Fail to send UDP data\n"); break;
            case(0): os_printf("espconn_send: Ok\n"); break;
            default: os_printf("espconn_send: UNDEFINED\n"); break;
        };
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

    // Connect to AP
    struct station_config config;
    os_memset(&config, 0, sizeof(struct station_config));
    os_memcpy(config.ssid, SSID, sizeof(SSID)-1);
    os_memcpy(config.password, PASSWORD, sizeof(PASSWORD)-1);

    wifi_station_set_config_current(&config);
    wifi_station_set_reconnect_policy(true);

    // Setup timer (2000ms, repeating)
    os_timer_setfn(&timer, (os_timer_func_t *)timer_interrupt, NULL);
    os_timer_arm(&timer, 1000, 1);
}