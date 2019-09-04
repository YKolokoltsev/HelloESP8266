
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