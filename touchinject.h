#ifndef _TOUCHINJECT_H
#define _TOUCHINJECT_H

#include <linux/input.h>
#include "conf.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <paths.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <linux/types.h>
#include <termios.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <cmath>
#include <algorithm>


 

#define EV(__TYP, __CODE, __VAL) \
		      (struct input_event){ .type = (__TYP), .code = (__CODE), .value = (__VAL) }
#define LEN_EV sizeof(struct input_event)
#define MDELAY(__MS)\
		      usleep((__MS)*1000)

//每个触摸点之间的时间间隔(ms)
#define G_DELAY 0.7

#include <linux/input.h>
#include <linux/uinput.h>
#include <thread>

struct TouchProxy {
    int phys_fd;   // upstream FD
    int v_fd;      // proxy output FD
    std::thread* loop_th=NULL;
    int max_x;
    int max_y;
    bool virtual_active=0;
    bool has_phy_finger=0;
    volatile bool safe_stop=0;
};

TouchProxy* init_touch_proxy(const char* phys_path,int dX=1000,int dY=1500);

int destroy_proxy(TouchProxy *tp);

void run_proxy_loop(TouchProxy* proxy);

// 供外部调用的实时映射接口
void set_virtual_touch_state(TouchProxy* proxy, int x, int y, bool down);

void cleanup_proxy(TouchProxy* proxy);

#endif // _TOUCHINJECT_H
