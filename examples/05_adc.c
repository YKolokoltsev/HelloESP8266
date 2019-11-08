/**
 * In this example we explore the possibilities of the ESP8266 ADC module.
 * The communication is the same as in the previous example - an UDP flood.
 * However here we use a shoutcast IP *.*.*.255 to be able to send the
 * same packets to many PCs at the same time.
 */

#define USE_US_TIMER

#include <osapi.h>
#include <user_interface.h>
#include <espconn.h>
#include <mem.h>

#define SSID "INFINITUMA8F2"
#define PASSWORD "1837497823"
static const uint8 r_ip[4] = {192, 168, 1, 67};
static const uint16 r_port = 1234;
static uint8* DATA_BUF;
//This length shell be multiple of 6 because each ADC measurement gives 2 bytes and
//local time point gives 4 bytes
const static uint16_t DATA_BUF_LEN = 192;

volatile static struct station_config config;
volatile static struct espconn conn;
volatile static esp_udp udp_proto_tx;
volatile static os_timer_t timer;

//buffer fill counter
volatile uint16 cnt_adc_buf = 0;

void
soft_timer_cb(void *arg) {

    // Read ADC to the packet buffer and send packet when the buffer is full

    if(cnt_adc_buf >= DATA_BUF_LEN-1){
        espconn_send((struct espconn *) &conn, DATA_BUF, DATA_BUF_LEN);
        cnt_adc_buf = 0;
    }else{

        // Read ADC data and store it in the packet buffer
        uint16 sample = system_adc_read();
        os_memcpy((void*)&DATA_BUF[cnt_adc_buf], &sample, 2);
        cnt_adc_buf += 2;

        // Read currect local timer counter and also store it to the buffer
        uint32_t t = system_get_time();
        os_memcpy((void*)&DATA_BUF[cnt_adc_buf], &t, 4);
        cnt_adc_buf += 4;
    }

    // Async hack to invoke this callback as fast as possible
    //os_timer_disarm((os_timer_t * ) & timer);
    //os_timer_arm_us((os_timer_t * ) & timer, 100, 1);

}

LOCAL
void
ICACHE_FLASH_ATTR
wifi_event_cb(System_Event_t *event){
    if(event->event != EVENT_STAMODE_GOT_IP) return;

    if(espconn_create((struct espconn*) &conn) == 0){
        os_printf("Starting transmission.\n");
        //TODO: The speed is critical - check this
        //TODO: SSID can be lost and does not recover - check this 
        os_timer_arm_us((os_timer_t * ) & timer, 3000, 1);
    }else{
        os_printf("Could not create UDP connection.\n");
    }
}

void
ICACHE_FLASH_ATTR
user_init() {

    // Allocate memory for the packet data
    DATA_BUF = os_malloc(DATA_BUF_LEN);

    // Setup Wi-Fi
    wifi_set_opmode_current(STATION_MODE);
    wifi_station_set_reconnect_policy(true);
    wifi_set_event_handler_cb(wifi_event_cb);

    // Setup timer, but not arm it
    os_timer_setfn((os_timer_t*)&timer, soft_timer_cb, NULL);
    system_timer_reinit();

    // Configure UDP connection
    os_memcpy((void*)udp_proto_tx.remote_ip, r_ip, 4);
    udp_proto_tx.remote_port = r_port;

    conn.type = ESPCONN_UDP;
    conn.state = ESPCONN_NONE;
    conn.proto.udp = (esp_udp *) &udp_proto_tx;

    // Prepare AP data
    os_memset((struct station_config*) &config, 0, sizeof(struct station_config));
    os_memcpy((void*)config.ssid, SSID, sizeof(SSID) - 1);
    os_memcpy((void*)config.password, PASSWORD, sizeof(PASSWORD) - 1);

    // Connect to AP
    wifi_station_set_config_current((struct station_config*) &config);
}

