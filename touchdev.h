#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
 
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
 
#define INPUT_B 1
 
#if INPUT_B
 
#endif 
volatile extern int uinp_fd; 
extern int dev_fd;
 
extern int global_tracking_id;
 
 
 int  device_writeEvent(int fd, uint16_t type, uint16_t keycode, int32_t value);


 
int createTouchScreen(int dwidth,int dheight);
 
 
 
void execute_sleep(int duration_msec);	
 
 void nvr_execute_touch(int fd,int startX,int startY,int endX,int endY );
 
#if INPUT_B
void touch_down(int fd,int x,int y,int slot,int trackingid);

 
#else
void touch_down(int fd,int x,int y,int slot); 
 
#endif 
 
void touch_up(int slot);

 
 
void sendScreenTouch(int startX,int startY,int endX,int endY); 
//	touch_down(0,i,i, 1,1); 
//	device_writeEvent(dev_fd, EV_SYN, SYN_REPORT, 0);
//	device_writeEvent(3, EV_SYN, SYN_REPORT, 0);
 
 
void touch_up();
