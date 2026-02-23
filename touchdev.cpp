// Original author: csdn.net @privileges
// Article: https://blog.csdn.net/u011578085/article/details/84991968

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
#include "touchdev.h" 
#define INPUT_B 1
 
#if INPUT_B
 
#endif 
 
int dev_fd=-1;
 
int global_tracking_id = 1;
 
 
int  device_writeEvent(int fd, uint16_t type, uint16_t keycode, int32_t value) {
    struct input_event ev;  
  
    memset(&ev, 0, sizeof(struct input_event));  
  
    ev.type = type;  
    ev.code = keycode;  
    ev.value = value;
    if (write(fd, &ev, sizeof(struct input_event)) < 0) {
		char * mesg = strerror(errno);
       printf("nibiru uinput errormag info :%s\n",mesg); 
        return 0;
    }  
 
    return 1;
   }
 
 
volatile int uinp_fd;
int createTouchScreen(int dwidth=1024,int dheight=2048)
{
		struct uinput_user_dev uinp;
		struct input_event event;
		uinp_fd = open("/dev/uinput", O_WRONLY|O_NONBLOCK);
		if(uinp_fd == 0) {
			printf("Unable to open /dev/uinput\n");
			return -1;
		}
 		printf("Opened /dev/uinput\n");
		dev_fd=uinp_fd;// 
				
		// configure touch device event properties
		memset(&uinp, 0, sizeof(uinp));
                //设备的别名
		strncpy(uinp.name, "WACOM", UINPUT_MAX_NAME_SIZE);
		uinp.id.version = 1;
		#if INPUT_B
		uinp.absmin[ABS_MT_SLOT] = 0;
		uinp.absmax[ABS_MT_SLOT] = 9;
		uinp.absmin[ABS_MT_TRACKING_ID] = 0;
		uinp.absmax[ABS_MT_TRACKING_ID] = 65535;//按键码ID累计叠加最大值
		ioctl (uinp_fd, UI_SET_ABSBIT, ABS_MT_SLOT);
		#endif
		 
 
		uinp.id.bustype = BUS_USB;
		//uinp.absmin[ABS_MT_SLOT] = 0;
		//uinp.absmax[ABS_MT_SLOT] = 9; // MT代表multi touch 多指触摸 最大手指的数量我们设置9
		uinp.absmin[ABS_MT_TOUCH_MAJOR] = 0;
		uinp.absmax[ABS_MT_TOUCH_MAJOR] = 15;
		uinp.absmin[ABS_MT_POSITION_X] = 0; // 屏幕最小的X尺寸
		uinp.absmax[ABS_MT_POSITION_X] = dwidth; // 屏幕最大的X尺寸
		uinp.absmin[ABS_MT_POSITION_Y] = 0; // 屏幕最小的Y尺寸
		uinp.absmax[ABS_MT_POSITION_Y] = dheight; //屏幕最大的Y尺寸
		uinp.absmin[ABS_MT_TRACKING_ID] = 0;
		//uinp.absmax[ABS_MT_TRACKING_ID] = 65535;//按键码ID累计叠加最大值
		uinp.absmin[ABS_MT_PRESSURE] = 0;   
		uinp.absmax[ABS_MT_PRESSURE] = 255;     //屏幕按下的压力值
 
		// Setup the uinput device
		// ioctl(uinp_fd, UI_SET_EVBIT, EV_KEY);   //该设备支持按键
		ioctl(uinp_fd, UI_SET_EVBIT, EV_REL);   //支持鼠标
    
		// Touch
		ioctl (uinp_fd, UI_SET_EVBIT,  EV_ABS); //支持触摸
		//ioctl (uinp_fd, UI_SET_ABSBIT, ABS_MT_SLOT);
		ioctl (uinp_fd, UI_SET_ABSBIT, ABS_MT_TOUCH_MAJOR);
		ioctl (uinp_fd, UI_SET_ABSBIT, ABS_MT_POSITION_X);
		ioctl (uinp_fd, UI_SET_ABSBIT, ABS_MT_POSITION_Y);
		ioctl (uinp_fd, UI_SET_ABSBIT, ABS_MT_TRACKING_ID);
		ioctl (uinp_fd, UI_SET_ABSBIT, ABS_MT_PRESSURE);    
		ioctl (uinp_fd, UI_SET_PROPBIT, INPUT_PROP_DIRECT);
	 
		ioctl (uinp_fd, UI_SET_KEYBIT, BTN_TOUCH);
		char str[20];
		memset(str,0,sizeof(str));
		sprintf(str,"%d",uinp_fd);
 
		// property_set("nvr_touch_screen_device",str);
		//printf("nvr touch screen device strfd = %s , id = %d\n",str ,uinp_fd);
    
		/* Create input device into input sub-system */
		write(uinp_fd, &uinp, sizeof(uinp));
		ioctl(uinp_fd, UI_DEV_CREATE);
		return 0;
 
}
 
 
 
 void execute_sleep(int duration_msec)
	{
	usleep(duration_msec*1000); 
	}
 
 void nvr_execute_touch(int fd,int startX,int startY,int endX,int endY )
	{	
 
		device_writeEvent(fd, EV_ABS, ABS_MT_TRACKING_ID, global_tracking_id++);
		//MT_TOOL_FINGER
		 startX= 0;
		 startY= 0;
		
		for(int i=0;i<1000;i++)
		{
		execute_sleep(2);
		device_writeEvent(fd, EV_KEY, BTN_TOUCH, 1);
		device_writeEvent(fd, EV_ABS, MT_TOOL_FINGER, 1);
		//ABS_X
		
		device_writeEvent(fd, EV_ABS, ABS_MT_POSITION_X, startX++);
		device_writeEvent(fd, EV_ABS, ABS_MT_POSITION_Y, startY++);
		device_writeEvent(fd, EV_ABS, ABS_MT_PRESSURE, 60);
		device_writeEvent(fd, EV_ABS, ABS_MT_TOUCH_MAJOR, 5);
				
		
	
 		endX++;
		endY++;
                //action_move事件
		
		 
                }
		device_writeEvent(fd, EV_SYN, SYN_REPORT, 0);
		//device_writeEvent(fd, EV_SYN, SYN_REPORT, 0);
		//device_writeEvent(fd, EV_ABS, ABS_MT_POSITION_X, endX);
		//device_writeEvent(fd, EV_ABS, ABS_MT_POSITION_Y, endY);
		//device_writeEvent(fd, EV_SYN, SYN_REPORT, 0);
                //action_up事件
		device_writeEvent(fd, EV_KEY, BTN_TOUCH, 0);
		device_writeEvent(fd, EV_ABS, ABS_MT_TRACKING_ID, -1);
		device_writeEvent(fd, EV_ABS, MT_TOOL_FINGER, 0);
                //事件发送完毕需要sync 
		device_writeEvent(fd, EV_SYN, SYN_REPORT, 0);
 
//
 		
 
		for(int i=0;i<0;i++)
		{execute_sleep(3);
		startX--;
		startY-=2;
		device_writeEvent(fd, EV_ABS, MT_TOOL_FINGER, 1);
		device_writeEvent(fd, EV_ABS, ABS_MT_POSITION_X, startX );
		device_writeEvent(fd, EV_ABS, ABS_MT_POSITION_Y, startY);
		device_writeEvent(fd, EV_ABS, ABS_MT_PRESSURE, 60);
		device_writeEvent(fd, EV_ABS, ABS_MT_TOUCH_MAJOR, 5);
		
 		endX++;
		endY++;
                //action_move事件
		device_writeEvent(fd, EV_KEY, BTN_TOUCH, 1);
		 
                }
		device_writeEvent(fd, EV_SYN, SYN_REPORT, 0);
		//device_writeEvent(fd, EV_SYN, SYN_REPORT, 0);
		//device_writeEvent(fd, EV_ABS, ABS_MT_POSITION_X, endX);
		//device_writeEvent(fd, EV_ABS, ABS_MT_POSITION_Y, endY);
		//device_writeEvent(fd, EV_SYN, SYN_REPORT, 0);
                //action_up事件
		device_writeEvent(fd, EV_ABS, ABS_MT_TRACKING_ID, -1);
		device_writeEvent(fd, EV_ABS, MT_TOOL_FINGER, 0);
                //事件发送完毕需要sync 
		device_writeEvent(fd, EV_SYN, SYN_REPORT, 0);
		device_writeEvent(fd, EV_KEY, BTN_TOUCH, 0);
 
 
	}
 
#if INPUT_B
void touch_down(int fd,int x,int y,int slot,int trackingid)
 
 
{
 
		fd = dev_fd;
       		execute_sleep(20);
		//ABS_MT_SLOT
 
		
		device_writeEvent(fd, EV_KEY, BTN_TOUCH, 1);
		device_writeEvent(fd, EV_ABS, MT_TOOL_FINGER, 1);
		
		device_writeEvent(fd, EV_ABS, ABS_MT_SLOT, slot);
		device_writeEvent(fd, EV_ABS, ABS_MT_TRACKING_ID, trackingid);
		
		device_writeEvent(fd, EV_ABS, ABS_MT_POSITION_X, x );
		device_writeEvent(fd, EV_ABS, ABS_MT_POSITION_Y, y );
		device_writeEvent(fd, EV_ABS, ABS_MT_PRESSURE, 60);
		device_writeEvent(fd, EV_ABS, ABS_MT_TOUCH_MAJOR, 5);
		
		//device_writeEvent(fd, EV_SYN, SYN_MT_REPORT, 0);		
 
 
	 	 
 
 
 
 
}
#else
 void touch_down(int fd,int x,int y,int slot)
 
{
		 
		 fd = uinp_fd;
       		//printf("touch screen fd = %d\n",fd);
 
	 
		execute_sleep(20);
		//device_writeEvent(fd, EV_ABS, ABS_MT_TRACKING_ID, global_tracking_id);
		
		device_writeEvent(fd, EV_KEY, BTN_TOUCH, 1);
		device_writeEvent(fd, EV_ABS, MT_TOOL_FINGER, 1);
		//ABS_X
		
		device_writeEvent(fd, EV_ABS, ABS_MT_POSITION_X, x );
		device_writeEvent(fd, EV_ABS, ABS_MT_POSITION_Y, y );
		device_writeEvent(fd, EV_ABS, ABS_MT_PRESSURE, 60);
		device_writeEvent(fd, EV_ABS, ABS_MT_TOUCH_MAJOR, 5);
		
		device_writeEvent(fd, EV_SYN, SYN_MT_REPORT, 0);		
 
 
	 	slot++;
 
	//device_writeEvent(fd, EV_SYN, SYN_REPORT, 0);
 
 
 
}
 
#endif 
 
void touch_up()
 
{	 
	int  fd =  dev_fd;
       	//printf("touch screen fd = %d\n",fd);
#if INPUT_B
	for(int i=1;i<6;i++)
{
	device_writeEvent(fd, EV_ABS, ABS_MT_SLOT, i);
	device_writeEvent(fd, EV_ABS, ABS_MT_TRACKING_ID, -1);
}	 
#else
	
	 device_writeEvent(fd, EV_KEY, BTN_TOUCH, 0);
	 device_writeEvent(fd, EV_ABS, MT_TOOL_FINGER, 0);
        //事件发送完毕需要sync 
	#endif 
	device_writeEvent(fd, EV_SYN, SYN_REPORT, 0);
	
 
}
 
 
void sendScreenTouch(int startX,int startY,int endX,int endY)
	{
	
		//char package_status[PROPERTY_VALUE_MAX]={0};
		//property_get("nvr_touch_screen_device",package_status,NULL);
		//int fd =  atoi(package_status);
      // 		printf("touch screen fd = %d\n",fd);
		nvr_execute_touch(uinp_fd,startX,startY,endX,endY );
		execute_sleep(20);
	}
 
 

