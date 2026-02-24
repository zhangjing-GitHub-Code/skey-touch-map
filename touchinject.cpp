#ifdef __clang__
    // 1. 保存当前的诊断设置状态
    #pragma clang diagnostic push 
    // 2. 忽略临时变量取地址的警告
    #pragma clang diagnostic ignored "-Waddress-of-temporary"
#endif
#include "touchinject.h"
#include <cmath>
#include <algorithm>


float map(float val,float ilb,float irb,float tlb,float trb){
if(tlb>trb)std::swap(tlb,trb);
if(ilb>irb)std::swap(ilb,irb);
return (val-ilb) * (trb - tlb) / (irb - ilb);
}
int imap(float val,float ilb,float irb,float tlb,float trb){
		  double r=map(val,ilb,irb,tlb,trb);
		  int ri=r;
		  if(r < (double)ri+0.5f)return ri;
		  return ri+1;
}

input_absinfo absX, absY;
int dx_max,dy_max;
bool axinfo_ok=1;
int initTouchInject(int dX,int dY){
    dx_max=dX,dy_max=dY;
    int fd=open("/dev/input/event6",O_RDWR|O_CREAT);
    if(fd<0)return fd;
    if (ioctl(fd, EVIOCGABS(ABS_X), &absX) < 0) {
        perror("Get X asix info failed.");
		  axinfo_ok=0;
    }

    // 3. 获取 Y 轴信息 (ABS_Y)
    if (ioctl(fd, EVIOCGABS(ABS_Y), &absY) < 0) {
        perror("Get Y axis info failed.");
		  axinfo_ok=0;
    }
    return fd;
}
// 封装一个点击函数
void tap(int fd, int x, int y)
{
    start_touch(fd,x,y);
    MDELAY(G_DELAY);
    end_touch(fd);
}
// 开始一次点击(手指接触)
void start_touch(int fd,int x, int y)
{
    sendev(fd, &EV(EV_ABS,ABS_MT_SLOT,9));
    // EV_KEY BTN_TOUCH DOWN
    sendev(fd, &EV(EV_KEY,BTN_TOUCH,1));
    sendev(fd, &EV(EV_KEY,BTN_TOOL_FINGER,1));
    // EV_ABS ABS_MT_TRACKING_ID
    sendev(fd, &EV(EV_ABS,ABS_MT_TRACKING_ID,0xf0));
    sendev(fd, &EV(EV_ABS,ABS_MT_PRESSURE,0x20));
    touch_point(fd,x,y);
}
// 结束点击,手指离开
void end_touch(int fd)
{
    sendev(fd, &EV(EV_ABS,ABS_MT_SLOT,9));
    sendev(fd, &EV(EV_ABS,ABS_MT_TRACKING_ID,-1));
    sendev(fd, &EV(EV_SYN,SYN_REPORT,0));
    // EV_ABS ABS_MT_TRACKING_ID (释放ID)
    //sendev(fd, &EV(EV_KEY,BTN_TOUCH,1));
    //sendev(fd, &EV(EV_KEY,BTN_TOOL_FINGER,1)); 
    //sendev(fd, &EV(EV_SYN,SYN_REPORT,0));
    sendev(fd, &EV(EV_ABS,ABS_MT_TRACKING_ID,-1));
    sendev(fd, &EV(EV_SYN,SYN_REPORT,0));
    // EV_KEY BTN_TOUCH UP
    //sendev(fd, &EV(EV_KEY,BTN_TOUCH,0));
    //sendev(fd, &EV(EV_KEY,BTN_TOOL_FINGER,0));
    // EV_SYN SYN_REPORT
    //sendev(fd, &EV(EV_SYN,SYN_REPORT,0));
}
//增加一个轨迹点(移动)
void touch_point(int fd,int x,int y){
    if(axinfo_ok){
        x=imap(x,0,dx_max,absX.minimum, absX.maximum);
        y=imap(y,0,dy_max,absY.minimum, absY.maximum);
    }
    sendev(fd, &EV(EV_ABS,ABS_MT_SLOT,9));
    sendev(fd, &EV(EV_ABS,ABS_MT_TRACKING_ID,0xf0));
    // EV_ABS ABS_MT_POSITION_X
    sendev(fd, &EV(EV_ABS,ABS_MT_POSITION_X,x));
    // EV_ABS ABS_MT_POSITION_Y
    sendev(fd, &EV(EV_ABS,ABS_MT_POSITION_Y,y));
    // EV_SYN SYN_REPORT 0
    sendev(fd, &EV(EV_SYN,SYN_REPORT,0));
    MDELAY(G_DELAY);
}
void sendev(int fd, struct input_event *ev)
{
    if(!(write(fd, ev, LEN_EV) == LEN_EV))
        printf("Error WriteDeviceFailed for %s","/dev/input/event6");
}
