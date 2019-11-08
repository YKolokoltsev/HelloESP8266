/**
 * In this example we connect to AP, and periodically trying to send
 * UDP packet by calling espconn_sendto. In the case if the packet was sent
 * successfully, this function returns 0 and calls sent_cb.
 *
 * UDP protocol does not support acknowledgment if the packet being sent was
 * received by the other computer or not. This protocol is faster than TCP,
 * and is perfect for continuous data flows. It supposes data loss and packets
 * rearrangement at the receiver. However it's usage require for some practice.
 *
 * When we call espconn_sendto it will put a packet into the output buffer of a WiFi module.
 * If at this moment we call espconn_sendto again, the previous packet will be
 * discarded and espconn_sendto will return an error. So, rising the
 * frequecy of espconn_sendto will produce lowering of the traffic speed.
 * Also, if we call espconn_sendto too slow (slower than the real
 * communication speed) it will also lower communication speed. So there exist some optimal
 * frequecy of the espconn_sendto calls, when average internal timeout of the
 * WiFi module packet buffer would coincide with the period of espconn_sendto
 * calls.
 *
 */
#define USE_US_TIMER

#include <osapi.h>
#include <user_interface.h>
#include <espconn.h>
#include <mem.h>

//TODO: Read this!
//http://smallbits.marshall-tribe.net/blog/2016/05/21/esp8266-networking-basics

#define SSID "INFINITUMA8F2"
#define PASSWORD "1837497823"
static const uint8 r_ip[4] = {192, 168, 1, 64};
static const uint16 r_port = 1234;
static uint8* DATA_BUF;
const static uint16_t DATA_BUF_LEN = 1024;

volatile static struct station_config config;
volatile static struct espconn conn;
// esp_udp is a alias to the struct called _esp_udp, an alias is already an identifier in C language
// so here is no "struct keyword"
volatile static esp_udp udp_proto_tx;
volatile static os_timer_t timer;

// statistics
volatile static uint32_t cnt_tx_sent = 0;
volatile static uint32_t cnt_timer = 0;
#define UDP_FLOOD
#include "common.c"
// #define LIFE_HACK

// ******************************************

/**
 * This function which is called by espconn_sendto when data was successfully sent.
 * It's not an interrupt of the WiFi subsystem. For example it is impossible to
 * call espconn_sendto from here, because it will produce recursion.
 *
 * @param arg pointer corresponding structure espconn
 */
void
sent_cb(void *arg){
    cnt_tx_sent++;
}

/**
 * The timer periodically calls espconn_sendto independently if
 * this function succeeds or not. Also it prints some statisctics over
 * UART0.
 *
 * @param arg
 */
void
soft_timer_cb(void *arg) {

    cnt_timer++;

    //Print statistics:
    if(cnt_timer >= 5000){
        print_statistics();
        cnt_tx_sent = 0;
        cnt_timer = 0;
    }


    //sending packet
    uint32_t t = system_get_time();
    sint8 res = espconn_send((struct espconn *) &conn, DATA_BUF, DATA_BUF_LEN);
    uint32_t dt = system_get_time() - t;

    collect_statistics(dt, res);

#ifdef LIFE_HACK
    os_timer_disarm((os_timer_t * ) & timer);
    os_timer_arm_us((os_timer_t * ) & timer, 100, 1);
#endif
}

LOCAL
void
ICACHE_FLASH_ATTR
wifi_event_cb(System_Event_t *event){
    if(event->event != EVENT_STAMODE_GOT_IP) return;

    //create UDP connection, it is required just once
    //the information that it is required to reconnect repeatedly
    //over the internet is not true
    if(espconn_create((struct espconn*) &conn) == 0){
        os_printf("Starting transmission.\n");

        //start transmission flow, the minimum value is 100us
        //for smaller values the chip will become unstable even
        //if soft_timer_cb has no code inside
        os_timer_arm_us((os_timer_t * ) & timer, 350, 1);

    }else{
        os_printf("Could not create UDP connection.\n");
    }
}

void
ICACHE_FLASH_ATTR
user_init() {

    // Allocate memory for the packet data (no data will be written,
    // no matter what we transmit, can be zeroes)
    DATA_BUF = os_malloc(DATA_BUF_LEN);

    // Setup Wi-Fi in a station-only current mode (do not update flash)
    // and say that we will reconnect if something
    wifi_set_opmode_current(STATION_MODE);
    wifi_station_set_reconnect_policy(true);
    wifi_set_event_handler_cb(wifi_event_cb);

    // Setup timer, but not arm it (see wifi_event_cb)
    os_timer_setfn((os_timer_t*)&timer, soft_timer_cb, NULL);
    system_timer_reinit();

    // Setup unique station hostname based on it's chip ID
    char hostname[32];
    os_memset(hostname, 0, sizeof(hostname));
    ets_sprintf(hostname, "ESP8266-%u", system_get_chip_id());
    wifi_station_set_hostname(hostname);

    // Configure UDP connection
    os_memcpy((void*)udp_proto_tx.remote_ip, r_ip, 4);
    udp_proto_tx.remote_port = r_port;

    conn.type = ESPCONN_UDP;
    conn.state = ESPCONN_NONE;
    conn.proto.udp = (esp_udp *) &udp_proto_tx;
    espconn_regist_sentcb( (struct espconn *) &conn, sent_cb);

    // AP to which we are connecting as a station (regular client WiFi)
    os_memset((struct station_config*) &config, 0, sizeof(struct station_config));
    os_memcpy((void*)config.ssid, SSID, sizeof(SSID) - 1);
    os_memcpy((void*)config.password, PASSWORD, sizeof(PASSWORD) - 1);

    // Connect to AP (after this we can wait for the EVENT_STAMODE_GOT_IP event).
    // At that event we can try to create an UDP connection.
    wifi_station_set_config_current((struct station_config*) &config);
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