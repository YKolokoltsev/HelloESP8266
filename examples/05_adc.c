/**
 * In this example we explore the possibilities of the ESP8266 ADC module.
 * The communication is nearly same as in the previous example - an UDP flood.
 * However here we use a shoutcast IP *.*.*.255 to be able to send the
 * same packets to many PCs at the same time (Classroom version).
 *
 * In this example we can observe a known ESP8266 problem - shot down of the WiFi interface
 * if time interval between the last system_adc_read call and espconn_send is less than 2-3ms.
 * This causes an uncomfortable situation - it is impossible to use internal ADC in the
 * continuous sampling mode at high sampling rates. The system_adc_read_fast is simply
 * not continuous by definition. Therefore, as it is mentioned in the official FAQ - an internal
 * ADC is imprecise and slow - it was added to be used for slow processes (less than 250 Hz in
 * continuous mode).
 *
 * In the case of triggered mode the system_adc_read_fast shell be used. It will be discussed
 * in the future examples. For the sync sampling an external ADC chip is required. ESP8266
 * SPI interface can support high speeds, and in the case of compression potentially can show
 * interesting results in continuous mode.
 */

#define USE_US_TIMER

#include <osapi.h>
#include <user_interface.h>
#include <espconn.h>
#include <mem.h>

#define DEBUG_WIFI_EVENT 1
#define DEBUG_ESPCONN 1
#include "common.c"

#define SSID "INFINITUMA8F2"
#define PASSWORD "1837497823"
static const uint8 r_ip[4] = {192, 168, 1, 255};
static const uint16 r_port = 1234;
static uint8* DATA_BUF;
// This length shell be multiple of 6 because each ADC measurement gives 2 bytes and
// local time point gives 4 bytes. If packet size is too big, the packet loss will increase
// significantly. If it is too small - inter-packet sampling will be ineffective for asynchronous
// frames with fast sampling inside.
const static uint16_t DATA_BUF_LEN = 192;

volatile static struct station_config config;
volatile static struct espconn conn;
volatile static esp_udp udp_proto_tx;
volatile static os_timer_t timer;

//buffer fill counter
volatile uint16 cnt_adc_buf = 0;
// ADC enable is required to stop samplig during Wi-Fi reconnection to AP,
// if it is not done, the automatic reconnection will fail due to WiFi instability
// during sampling.
volatile bool b_adc_enable = false;


void
soft_timer_cb(void *arg) {

    // Disarm timer to avoid overlaps of this interrupt processing with itself
    os_timer_disarm((os_timer_t * ) & timer);

    // Read ADC to the packet buffer and send packet when the buffer is full

    if(cnt_adc_buf >= DATA_BUF_LEN-1){
        // Give the FR module to recover itself before sending a new packet.
        // Experiments show that 3ms seems is near the minimal stable value.
        os_delay_us(3000);
        espconn_send((struct espconn *) &conn, DATA_BUF, DATA_BUF_LEN);
        cnt_adc_buf = 0;
    }else {

        // Read ADC data and store it in the packet buffer
        uint16 sample = 0;

        if(b_adc_enable)
            sample = system_adc_read();

        os_memcpy((void*)&DATA_BUF[cnt_adc_buf], &sample, 2);
        cnt_adc_buf += 2;

        // Read currect local timer counter and also store it to the buffer.
        // The timer event appears with an error that is to be estimated based
        // on this data.
        uint32_t t = system_get_time();
        os_memcpy((void*)&DATA_BUF[cnt_adc_buf], &t, 4);
        cnt_adc_buf += 4;
    }

    // invoke the next timer event as fast as possible (0x64 is the minimum value permitted)
    os_timer_arm_us((os_timer_t * ) & timer, 100, 1);
}

LOCAL
void
ICACHE_FLASH_ATTR
wifi_event_cb(System_Event_t *event){

    debug_wifi_event(event->event);

    if(event->event == EVENT_STAMODE_GOT_IP){

        // During reconnection this function can return ESPCONN_ISCONN
        sint8 res = espconn_create((struct espconn*) &conn);

        if(res == 0 || res ==  ESPCONN_ISCONN) {
            os_printf("Starting transmission.\n");
            // Generate first timer event after 1ms, within the timer event
            // it will be disarmed immediately, so this 1ms is not a sampling period
            os_timer_arm_us((os_timer_t * ) & timer, 1000, 1);
            b_adc_enable = true;
        }else{
            os_printf("Could not create UDP connection.\n");
            b_adc_enable = false;
        }

    }else if(event->event == EVENT_STAMODE_DISCONNECTED){
        b_adc_enable = false;
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
    wifi_set_sleep_type(NONE_SLEEP_T); //todo: check

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

