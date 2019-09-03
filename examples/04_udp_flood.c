/**
 * In this example we connect to the AP, and trying to send
 * UDP packets. The UDP protocol does not support acknowledgment
 * if the packet being sent was received by the other computer or not.
 * This protocol is faster than TCP, and is a good way to start with.
 * Also
 */
#define USE_US_TIMER

#include <osapi.h>
#include <user_interface.h>
#include <espconn.h>

//TODO: Read this!
//http://smallbits.marshall-tribe.net/blog/2016/05/21/esp8266-networking-basics

#define SSID "INFINITUMA8F2"
#define PASSWORD "1837497823"
static const uint8 r_ip[4] = {192, 168, 1, 64};
static const uint16 r_port = 1234;
static uint8 DATA_BUF[] = "ABCD";

volatile static struct station_config config;
volatile static struct espconn conn;
// esp_udp is a alias to the struct called _esp_udp, an alias is already an identifier in C language
// so here is no "struct keyword"
volatile static esp_udp udp_proto_tx;
volatile uint32_t cnt_timer_delay_us = 50;
volatile uint32_t cnt_timer_passed = 0;
volatile uint32_t cnt_packets_sent = 0;

// ******************************************

static os_timer_t timer; //volatile

void udp_tx_data() {

    switch(espconn_create((struct espconn*) &conn)){
        // all fine, can send another packet.
        // However, may be that timer was too slow -> decreasind timer delay.
        case(0):{
            espconn_sendto((struct espconn *) &conn, DATA_BUF, sizeof(DATA_BUF));
            espconn_delete((struct espconn *) &conn);
            cnt_timer_delay_us = cnt_timer_delay_us > 5 ? cnt_timer_delay_us-1 : 5;
            os_timer_arm_us(&timer, cnt_timer_delay_us, false);
            break;
        }
        // The data is transmitting at the moment, so that timer was too fast
        // increasing timer delay
        case(ESPCONN_ISCONN): {
            cnt_timer_delay_us = cnt_timer_delay_us < 2000 ? cnt_timer_delay_us+1 : 2000;
            os_timer_arm_us(&timer, cnt_timer_delay_us, false);
            break;
        }
        case(ESPCONN_MEM): os_printf("ESPCONN_MEM\n"); break;
        case(ESPCONN_ARG): os_printf("ESPCONN_ARG\n"); break;
        default: os_printf("ESPCONN_UNDEF\n"); break;
    }
}

/**
 * This function which will be called back when data are successfully sent.
 *
 * @param arg pointer corresponding structure espconn
 */
void
sent_interrupt(void *arg){
    cnt_packets_sent++;
}

/**
 * The timer works as an UDP transmission watchdog
 * @param arg
 */
void
timer_cb(void *arg) {
    //First print statistics:
    cnt_timer_passed += cnt_timer_delay_us;
    if(cnt_timer_passed >= 100000){
        os_printf("Packets sent: %d. Current delay: %dus, Current time: %d\n",
                cnt_packets_sent, cnt_timer_delay_us, cnt_timer_passed);
        cnt_packets_sent = 0;
        cnt_timer_passed = 0;
    }


    // Even do not try to do anything if we are disconnected from AP. Everything will
    // repeat after automatic reconnect. However, if we are connected, we try to
    // to transmit packet.
    if(wifi_station_get_connect_status() == STATION_GOT_IP){
        udp_tx_data();
    }
}

LOCAL
void
ICACHE_FLASH_ATTR
wifi_event_cb(System_Event_t *event){
    if(event->event != EVENT_STAMODE_GOT_IP) return;
    //start transmission in 1 sec
    os_printf("Transmission will be restarted in 1 sec.\n");
    os_timer_arm_us((os_timer_t*) &timer, 100000, false);
}

void
ICACHE_FLASH_ATTR
user_init() {
    // Setup Wi-Fi in a station-only current mode (do not update flash)
    // and say that we will reconnect if something
    wifi_set_opmode_current(STATION_MODE);
    wifi_station_set_reconnect_policy(true);
    wifi_set_event_handler_cb(wifi_event_cb);

    // Setup unique station hostname based on it's chip ID
    char hostname[32];
    os_memset(hostname, 0, sizeof(hostname));
    ets_sprintf(hostname, "ESP8266-%u", system_get_chip_id());
    wifi_station_set_hostname(hostname);

    // Prepare data for the UDP connection
    os_memcpy((void*)udp_proto_tx.remote_ip, r_ip, 4);
    udp_proto_tx.remote_port = r_port;

    conn.type = ESPCONN_UDP;
    conn.state = ESPCONN_NONE;
    conn.proto.udp = (esp_udp *) &udp_proto_tx;
    espconn_regist_sentcb( (struct espconn *) &conn, sent_interrupt);

    // Prepare AP data
    os_memset((struct station_config*) &config, 0, sizeof(struct station_config));
    os_memcpy((void*)config.ssid, SSID, sizeof(SSID) - 1);
    os_memcpy((void*)config.password, PASSWORD, sizeof(PASSWORD) - 1);

    // Connect to AP (after this we can wait for the EVENT_STAMODE_GOT_IP event).
    // At that event we can try to create an UDP connection.
    wifi_station_set_config_current((struct station_config*) &config);


    // Setup timer just callback function for the timer, that one is constant
    system_timer_reinit();
    os_timer_setfn(&timer, (os_timer_func_t *) timer_cb, NULL);
}

/*
 * TODO:
 * In the ESP8266 <-> AP channel can happen packet loss due to
 * many reasons. The ESP8266 chip can fail to send the packet on a
 * hardware level. The packet may be lost due to RF noise, and this effect
 * depends strongly from the distance to AP. The AP may be busy.
 *
 * 1.: Measure packet sending statistics using the local timer (take care that
 * local timers are cyclic), and report per second of the data being sent in Bytes/sec.
 * Do not take into account for the datagram and hardware level data being sent.
 *
 * 2.: Measure the same speed (in Bytes/sec) at the remote point. Make first comparison.
 *
 * 3.: Plot dependency graphic of the tr/tx speeds (double plot one on another) on the
 * Size of the packet.
 *
 * 4.: Find and fix the packet size when tx speed is at its maximum for the distance of 0m from AP,
 * and make another plot of that speed depending from the increasing distance to AP.
 *
 * Discuss the results in a report (make sure to specify your AP model and absence of the other considerable traffic
 * in your local network)
 */