
void
ICACHE_FLASH_ATTR
printFloat(float val, char *buff) {

    char smallBuff[16];
    int val1 = (int) val;
    unsigned int val2;
    if (val < 0) {
        val2 = (int) (-100.0 * val) % 100;
    } else {
        val2 = (int) (100.0 * val) % 100;
    }

    os_sprintf(smallBuff, "%i.%02u", val1, val2);

    strcat(buff, smallBuff);
}

#ifdef UDP_FLOOD

volatile static uint32_t cnt_err[6] = {0,0,0,0,0,0};
volatile static uint32_t cnt_rtc[6] = {0,0,0,0,0,0};

volatile static uint32_t t_stat_prev_us = 0;
volatile static uint32_t dt_stat_us = 0;

void
ICACHE_FLASH_ATTR
collect_statistics(uint32_t dt, sint8 res){
    switch(res){
        case 0: //OK
            cnt_err[0]++; cnt_rtc[0] += dt; break;
        case ESPCONN_IF:
            cnt_err[1]++; cnt_rtc[1] += dt; break;
        default:
            cnt_err[2]++; cnt_rtc[2] += dt; break;
    }
}

void
ICACHE_FLASH_ATTR
print_statistics(){

    // Fill floats manually (sorry, ESP API does not support printf for floats)
    char str_buf[5][20];
    for(int i = 0; i < 3; i++){
        os_memset((void*)str_buf[i], 0, sizeof(str_buf[i]));
        if(cnt_err[i] > 0) {
            printFloat((float) cnt_rtc[i] / (float) cnt_err[i], str_buf[i]);
        }else{
            printFloat(0.0, str_buf[i]);
        }
    }

    // Calc transmission speed in bits per second
    // UDP packet has 8 bytes header and the rest is its data. Each byte has 8 bits

    if(t_stat_prev_us > 0)
        dt_stat_us = system_get_time() - t_stat_prev_us;

    t_stat_prev_us = system_get_time();

    os_memset((void*)str_buf[3], 0, sizeof(str_buf[3]));
    if(dt_stat_us > 0){
        uint32_t tx_bits = (8 + DATA_BUF_LEN)*cnt_tx_sent*8;
        printFloat(1.0e3*(float) tx_bits / (float) dt_stat_us, str_buf[3]);
    }else{
        printFloat(0.0, str_buf[3]);
    }

    // Calc lost %
    os_memset((void*)str_buf[4], 0, sizeof(str_buf[4]));
    uint32_t total = cnt_err[0] + cnt_err[1] + cnt_err[2] + cnt_err[3] + cnt_err[4] + cnt_err[5];
    if(total > 0){
        printFloat(100.0*(float) (1.0 - (float) cnt_tx_sent / (float) total), str_buf[4]);
    }else{
        strcat(str_buf[4], "n/a ");
    }


    // Print table line
    os_printf("Total packets: %5d, "
              "sent_cb: %5d, "
              "OK: %5d (%s us/pkt), "
              "IF: %5d (%s us/pkt), "
              "Other: %2d (%s us/pkt), "
              "Speed: %s kbits/sec, "
              "Lost: %s %%\n",
              total,
              cnt_tx_sent,
              cnt_err[0], str_buf[0],
              cnt_err[1], str_buf[1],
              cnt_err[2], str_buf[2],
              str_buf[3], str_buf[4]);

    
    os_memset((void*)cnt_err, 0, sizeof(cnt_err));
    os_memset((void*)cnt_rtc, 0, sizeof(cnt_rtc));
}

#endif

#ifdef DEBUG_WIFI_EVENT
void debug_wifi_event(uint32 event){
    switch(event){
        case (EVENT_STAMODE_CONNECTED):
            os_printf("*** wifi_event_cb : EVENT_STAMODE_CONNECTED\n");
            break;
        case (EVENT_STAMODE_DISCONNECTED):
            os_printf("*** wifi_event_cb : EVENT_STAMODE_DISCONNECTED\n");
            break;
        case (EVENT_STAMODE_AUTHMODE_CHANGE):
            os_printf("*** wifi_event_cb : EVENT_STAMODE_AUTHMODE_CHANGE\n");
            break;
        case (EVENT_STAMODE_GOT_IP):
            os_printf("*** wifi_event_cb : EVENT_STAMODE_GOT_IP\n");
            break;
        case (EVENT_STAMODE_DHCP_TIMEOUT):
            os_printf("*** wifi_event_cb : EVENT_STAMODE_DHCP_TIMEOUT\n");
            break;
        case (EVENT_SOFTAPMODE_STACONNECTED):
            os_printf("*** wifi_event_cb : EVENT_SOFTAPMODE_STACONNECTED\n");
            break;
        case (EVENT_SOFTAPMODE_STADISCONNECTED):
            os_printf("*** wifi_event_cb : EVENT_SOFTAPMODE_STADISCONNECTED\n");
            break;
        case (EVENT_SOFTAPMODE_PROBEREQRECVED):
            os_printf("*** wifi_event_cb : EVENT_SOFTAPMODE_PROBEREQRECVED\n");
            break;
        case (EVENT_OPMODE_CHANGED):
            os_printf("*** wifi_event_cb : EVENT_OPMODE_CHANGED\n");
            break;
        case (EVENT_MAX):
            os_printf("*** wifi_event_cb : EVENT_MAX\n");
            break;
        default:
            os_printf("*** wifi_event_cb : UNKNOWN\n");
            break;
    }
}
#endif

#ifdef DEBUG_ESPCONN

void debug_espconn(struct espconn* p_conn){

    switch(p_conn->type){
        case(ESPCONN_INVALID):
            os_printf("*** espconn.type: ESPCONN_INVALID\n");
            break;
        case(ESPCONN_TCP):
            os_printf("*** espconn.type: ESPCONN_TCP\n");
            break;
        case(ESPCONN_UDP):
            os_printf("*** espconn.type: ESPCONN_UDP\n");
            break;
        default:
            os_printf("*** espconn.type: UNKNOWN\n");
            break;
    }

    switch(p_conn->state){
        case(ESPCONN_NONE):
            os_printf("*** espconn.state: ESPCONN_NONE\n");
            break;
        case(ESPCONN_WAIT):
            os_printf("*** espconn.state: ESPCONN_WAIT\n");
            break;
        case(ESPCONN_LISTEN):
            os_printf("*** espconn.state: ESPCONN_LISTEN\n");
            break;
        case(ESPCONN_CONNECT):
            os_printf("*** espconn.state: ESPCONN_CONNECT\n");
            break;
        case(ESPCONN_WRITE):
            os_printf("*** espconn.state: ESPCONN_WRITE\n");
            break;
        case(ESPCONN_READ):
            os_printf("*** espconn.state: ESPCONN_READ\n");
            break;
        case(ESPCONN_CLOSE):
            os_printf("*** espconn.state: ESPCONN_CLOSE\n");
            break;
        default:
            os_printf("*** espconn.state: UNKNOWN\n");
            break;
    }

    if(p_conn->type == ESPCONN_UDP){
        os_printf("*** udp local:  %d.%d.%d.%d : %d\n", IP2STR(p_conn->proto.udp->local_ip),
                p_conn->proto.udp->local_port);
        os_printf("*** udp remote: %d.%d.%d.%d : %d\n", IP2STR(p_conn->proto.udp->remote_ip),
                p_conn->proto.udp->remote_port);
    }

    if(p_conn->type == ESPCONN_TCP){
        os_printf("*** tcp local:  %d.%d.%d.%d : %d\n", IP2STR(p_conn->proto.tcp->local_ip),
                p_conn->proto.tcp->local_port);
        os_printf("*** tcp remote: %d.%d.%d.%d : %d\n", IP2STR(p_conn->proto.tcp->remote_ip),
                p_conn->proto.tcp->remote_port);

        if(p_conn->proto.tcp->connect_callback){
            os_printf("*** tcp connect_callback: yes\n");
        }else{
            os_printf("*** tcp connect_callback: no\n");
        }

        if(p_conn->proto.tcp->reconnect_callback){
            os_printf("*** tcp reconnect_callback: yes\n");
        }else{
            os_printf("*** tcp reconnect_callback: no\n");
        }

        if(p_conn->proto.tcp->disconnect_callback){
            os_printf("*** tcp disconnect_callback: yes\n");
        }else{
            os_printf("*** tcp disconnect_callback: no\n");
        }

        if(p_conn->proto.tcp->write_finish_fn){
            os_printf("*** tcp write_finish_fn: yes\n");
        }else{
            os_printf("*** tcp write_finish_fn: no\n");
        }
    }

    if(p_conn->recv_callback){
        os_printf("*** conn recv_callback: yes\n");
    }else{
        os_printf("*** conn recv_callback: no\n");
    }

    if(p_conn->sent_callback){
        os_printf("*** conn sent_callback: yes\n");
    }else{
        os_printf("*** conn sent_callback: no\n");
    }

    os_printf("*** espconn.link_cnt: %d\n", p_conn->link_cnt);

    if(p_conn->reverse){
        os_printf("*** conn reverse: yes\n");
    }else{
        os_printf("*** conn reverse: no\n");
    }
}

#endif