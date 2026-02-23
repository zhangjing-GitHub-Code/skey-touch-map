#include "touchdev.h"

 int main()
{
 
createTouchScreen();
//execute_sleep(100);
 
//for(int i=0;i<100000;i++)
	if(0)
	sendScreenTouch(0,0,300,300);
	else
	for(int  i=0;i<20;i++) 	
	{
#if INPUT_B
 
	touch_down(0,i,i, 1,1);
 
	touch_down(0,2*i,i, 2,2);
	touch_down(0,3*i,i, 3,3);
	touch_down(0,5*i,i, 4,4);
	touch_down(0,4*i,i, 5,5);
 
	device_writeEvent(dev_fd, EV_SYN, SYN_REPORT, 0);
#else
	touch_down(0,i,i, 1);
	touch_down(0,2*i,i, 1);
	touch_down(0,3*i,i, 1);
	touch_down(0,4*i,i, 1);
	if(i<100)
	{
	touch_down(0,5*i,i, 1);
	//if(i==99)
		//touch_up();
	}
	device_writeEvent(3, EV_SYN, SYN_REPORT, 0);
	#endif 
	}
 
 
touch_up();
 
}

