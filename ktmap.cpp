#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <errno.h>
#include "conf.h"
#include "touchdev.h"
#include "dconfd.h"
#include "ipcdef.h"

volatile int TARGET_X;                 // Dummy触摸点 X 坐标
volatile int TARGET_Y;                // Dummy触摸点 Y 坐标

char* find_keyev(const char* name){
		  char devname[64];
		  char* fpath=(char*)malloc(256);
		  memset(fpath,0,255);
		  for(int i=0;i<9;++i){
		      sprintf(fpath,"/dev/input/event%d\0",i);
				int fd=open(fpath,O_RDONLY);
				if(fd<0)continue;
				ioctl(fd,EVIOCGNAME(63),devname);
				close(fd);
				printf("[D] Compare %d name '%s' with '%s'\n",i,devname,name);
				if(strcmp(devname,name)==0) return fpath;
		  }
		  free(fpath);
		  return KEYEV_FALLBK;
}

void handle_coord(int sig, siginfo_t *info, void *context) {
		  // printf("HANDLING SIG%d\n",sig);
    if (sig == SIG_UPDATE_COORD) {
        // 从 sigval_int 中解包坐标 (高16位为X，低16位为Y)
        int packed_val = info->si_value.sival_int;
        TARGET_X = (packed_val >> 16) & 0xFFFF;
        TARGET_Y = packed_val & 0xFFFF;
		  printf("Set coord: %d,%d\n",TARGET_X,TARGET_Y);
    }
}


int main() {
		  TARGET_X=500,TARGET_Y=700;
    int key_fd, uinput_fd;
    struct uinput_setup usetup;
    struct input_event ev;
	 char* keyev_path=find_keyev(KEY_DEV_NAME);
	 printf("Found %s as key event target.\n",keyev_path);
    // 1. 打开物理按键设备并独占 (屏蔽系统音量功能)
    key_fd = open(keyev_path, O_RDONLY);
    if (key_fd < 0) {
        perror("无法打开按键设备，请确认路径及Root权限");
        return 1;
    }
    // EVIOCGRAB 为 1 表示独占，系统将无法捕获该设备的任何事件
    if (ioctl(key_fd, EVIOCGRAB, 1) < 0) {
        perror("无法拦截按键(Grab Failed)");
        return 1;
    }
	 if(createTouchScreen(MAX_X,MAX_Y)<0){
				perror("Create Uinput failed...");
	 }
struct sigaction sa;
sa.sa_sigaction = handle_coord;
sa.sa_flags = SA_SIGINFO;
sigaction(SIG_UPDATE_COORD, &sa, NULL);
    int cpid=fork();
	 // printf("FORK RETS %d, my parent %d\n",cpid,getppid());
	 if(cpid==0){
        printf("Conf watcher spwaned.\n");
		  watch_conf();
		  printf("Conf watcher exitting...\n");
		  return 0;
	 }else if(cpid>0){
		  printf("Main daemon ready.\n");
	 }else{
        printf("Fork failed: %d!\n",cpid);
		  exit(1);
	 }
    // 3. 事件处理循环
	 bool prevfail=0;
    while (1) {
        if(read(key_fd, &ev, sizeof(ev))<=0){
					 if(errno==EINTR)continue;
				if(prevfail)break;
				prevfail=1;
				continue;
 	     }else prevfail=0;
        // KEY_VOLUMEUP 代码通常为 115
		  // printf("TYPE: %d, CODE: %d, VAL: %d\n",ev.type,ev.code,ev.value);
        if (ev.type == EV_KEY && ev.code == 115) {
					 // printf("VUP KEY: %d\n",ev.value);
            if (ev.value == 1) { // 按下
                // 模拟多点触控 Type B 协议
#if INPUT_B
 
					 	touch_down(0,TARGET_X,TARGET_Y, 1,1);
	device_writeEvent(dev_fd, EV_SYN, SYN_REPORT, 0);
#else
					 	touch_down(0,TARGET_X,TARGET_Y, 1);
	device_writeEvent(3, EV_SYN, SYN_REPORT, 0);
	#endif 
            } 
            else if (ev.value == 0) { // 松开
					 touch_up();
            }
        }
    }

    ioctl(key_fd, EVIOCGRAB, 0);
    ioctl(uinput_fd, UI_DEV_DESTROY);
    close(key_fd);
    close(uinput_fd);
    return 0;
}

