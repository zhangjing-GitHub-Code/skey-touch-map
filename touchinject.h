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
 

#define EV(__TYP, __CODE, __VAL) \
		      (struct input_event){ .type = (__TYP), .code = (__CODE), .value = (__VAL) }
#define LEN_EV sizeof(struct input_event)
#define MDELAY(__MS)\
		      usleep((__MS)*1000)

//每个触摸点之间的时间间隔(ms)
#define G_DELAY 0.7


int initTouchInject(int dX=1000,int dY=1500);
void tap(int fd, int x, int y);
void start_touch(int fd,int x, int y);
void end_touch(int fd);
void touch_point(int fd,int x,int y);
void sendev(int fd, struct input_event *ev);
