#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <thread>
#include <errno.h>
#include "conf.h"
#include "touchinject.h"
#include "dconfd.h"
#include "ipcdef.h"

volatile short TARGET_X, TARGET_Y;
int key_fd, uinput_fd;
TouchProxy* proxy = NULL;

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
void cleanup(int signal=0){
    destroy_proxy(proxy);
	 free(proxy);
    ioctl(key_fd, EVIOCGRAB, 0);
    ioctl(uinput_fd, UI_DEV_DESTROY);
    close(key_fd);
    close(uinput_fd);
	 exit(0);
}

int tdfd=-1;
int main() {
		  TARGET_X=500,TARGET_Y=700;
    struct uinput_setup usetup;
    struct input_event ev;
	 char* keyev_path=find_keyev(KEY_DEV_NAME);
	 printf("Found %s as key event target.\n",keyev_path);
    // 1. Open S-Key exclusively (Block original function)
    key_fd = open(keyev_path, O_RDONLY);
    if (key_fd < 0) {
        perror("Failed to open S-Key event, check permission/path!");
        return 1;
    }
    // EVIOCGRAB==1 meaning exclusive
    if (ioctl(key_fd, EVIOCGRAB, 1) < 0) {
        perror("GRAB S-Key Failed");
        return 1;
    }
    proxy = init_touch_proxy("/dev/input/event6",MAX_X,MAX_Y); 
	 if(proxy==NULL){
				perror("Create uInput proxy failed...");
				return 1;
	 }
	 // printf("[D] T FD %d, err%d\n",proxy,errno);
    struct sigaction sa;
    sa.sa_sigaction = handle_coord;
    sa.sa_flags = SA_SIGINFO;
    sigaction(SIG_UPDATE_COORD, &sa, NULL);
	 signal(SIGSTOP,cleanup);
	 signal(SIGINT,cleanup);
	 signal(SIGSEGV,cleanup);
	 signal(SIGKILL,cleanup);

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
        printf("Fork failed: %d err %s!\n",cpid,strerror(errno));
		  exit(1);
	 }

	 std::thread proxy_thread(run_proxy_loop, proxy);
	 proxy_thread.detach();
	 proxy->loop_th=&proxy_thread;
	 bool prevfail=0;
	 try{
    while (1) {
        if(read(key_fd, &ev, sizeof(ev))<=0){
		      if(errno==EINTR)continue;
		      printf("Error while reading key event: %s\n",strerror(errno));
				if(prevfail){
					 printf("Report problem! exitting...");
					 break;
		      }
				prevfail=1;
				continue;
 	     }else prevfail=0;
        // KEY_VOLUMEUP 代码通常为 115
		  // printf("TYPE: %d, CODE: %d, VAL: %d\n",ev.type,ev.code,ev.value);
        if (ev.type == EV_KEY && ev.code == 115) {
					 // printf("VUP KEY: %d\n",ev.value);
            if (ev.value == 1) { // 按下
					 //start_touch(tdfd,TARGET_X,TARGET_Y);
					 set_virtual_touch_state(proxy, TARGET_X, TARGET_Y, true);
            } 
            else if (ev.value == 0) { // 松开
					 set_virtual_touch_state(proxy, TARGET_X, TARGET_Y, false);
					 //end_touch(tdfd);
            }
        }
    }
	 }catch(...){
		  printf("GET EXCEPTION\n");
	 }
	 cleanup();
    return 0;
}

