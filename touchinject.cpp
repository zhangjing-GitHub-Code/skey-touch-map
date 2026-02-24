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




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>

TouchProxy* init_touch_proxy(const char* phys_path,int dX,int dY) {
    TouchProxy* proxy = (TouchProxy*)calloc(1, sizeof(TouchProxy));
    proxy->phys_fd = open(phys_path, O_RDONLY);
    if (proxy->phys_fd < 0) return NULL;
    dx_max=dX,dy_max=dY;
    if (ioctl(proxy->phys_fd, EVIOCGABS(ABS_X), &absX) < 0) {
        perror("Get X asix info failed.");
		  axinfo_ok=0;
    }

    // 3. 获取 Y 轴信息 (ABS_Y)
    if (ioctl(proxy->phys_fd, EVIOCGABS(ABS_Y), &absY) < 0) {
        perror("Get Y axis info failed.");
		  axinfo_ok=0;
    }


    // 1. 独占物理屏幕
    if (ioctl(proxy->phys_fd, EVIOCGRAB, 1) < 0) return NULL;

    proxy->v_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    
    // 2. 反检测：克隆物理设备所有属性
    char name[64];
    struct input_id id;
    unsigned char propbit[256];
    
    ioctl(proxy->phys_fd, EVIOCGNAME(sizeof(name)), name);
    ioctl(proxy->phys_fd, EVIOCGID, &id);
    ioctl(proxy->phys_fd, EVIOCGPROP(sizeof(propbit)), propbit);

    // 设置基本位
    ioctl(proxy->v_fd, UI_SET_EVBIT, EV_SYN);
    ioctl(proxy->v_fd, UI_SET_EVBIT, EV_KEY);
    ioctl(proxy->v_fd, UI_SET_EVBIT, EV_ABS);

    // 克隆所有 Key (BTN_TOUCH, BTN_TOOL_FINGER 等)
    unsigned char keybit[256];
    ioctl(proxy->phys_fd, EVIOCGBIT(EV_KEY, sizeof(keybit)), keybit);
    for (int i = 0; i < KEY_MAX; i++) {
        if (keybit[i / 8] & (1 << (i % 8))) ioctl(proxy->v_fd, UI_SET_KEYBIT, i);
    }
	 ioctl(proxy->v_fd, UI_SET_KEYBIT, BTN_TOUCH);
    ioctl(proxy->v_fd, UI_SET_KEYBIT, BTN_TOOL_FINGER);

    // 克隆所有属性 (INPUT_PROP_DIRECT 等)
    for (int i = 0; i < INPUT_PROP_MAX; i++) {
        if (propbit[i / 8] & (1 << (i % 8))) ioctl(proxy->v_fd, UI_SET_PROPBIT, i);
    }

    // 3. 克隆绝对轴极值 (ABS_MT_POSITION_X/Y 等)
    struct uinput_user_dev uud;
    memset(&uud, 0, sizeof(uud));
    strncpy(uud.name, name, UINPUT_MAX_NAME_SIZE);
    uud.id = id;

    for (int i = 0; i < ABS_CNT; i++) {
        struct input_absinfo abs;
        if (ioctl(proxy->phys_fd, EVIOCGABS(i), &abs) == 0) {
            ioctl(proxy->v_fd, UI_SET_ABSBIT, i);
            uud.absmax[i] = abs.maximum;
            uud.absmin[i] = abs.minimum;
            uud.absfuzz[i] = abs.fuzz;
            uud.absflat[i] = abs.flat;
            if (i == ABS_MT_POSITION_X) proxy->max_x = abs.maximum;
            if (i == ABS_MT_POSITION_Y) proxy->max_y = abs.maximum;
        }
    }

    write(proxy->v_fd, &uud, sizeof(uud));
    ioctl(proxy->v_fd, UI_DEV_CREATE);
    return proxy;
}

void set_virtual_touch_state(TouchProxy* proxy, int x, int y, bool down) {
    if(axinfo_ok){
				//printf("Trans %d,%d ->",x,y);
        x=imap(x,0,dx_max,absX.minimum, absX.maximum);
        y=imap(y,0,dy_max,absY.minimum, absY.maximum);
				//printf(" %d,%d; map range[%d,%d] -> [%d,%d]\n",x,y,
				//					 dx_max,dy_max,absY.maximum,absY.maximum);
    }
	 input_absinfo sinfo;
	 ioctl(proxy->phys_fd, EVIOCGABS(ABS_MT_SLOT), &sinfo);

    proxy->virtual_active = down;
    // 使用 Slot 9 避免与物理手指冲突
    struct input_event evs[16];
    int i = 0;
    evs[i++] = {{0,0}, EV_ABS, ABS_MT_SLOT, 9};
    if (down) {
        evs[i++] = {{0,0}, EV_ABS, ABS_MT_TRACKING_ID, 0xfe};
        evs[i++] = {{0,0}, EV_ABS, ABS_MT_PRESSURE, 0x20};
        evs[i++] = {{0,0}, EV_ABS, ABS_MT_POSITION_X, x};
        evs[i++] = {{0,0}, EV_ABS, ABS_MT_POSITION_Y, y};
        // 如果物理手指为0，主动补发 BTN_TOUCH 1
        if (proxy->has_phy_finger == 0) {
            evs[i++] = {{0,0}, EV_KEY, BTN_TOUCH, 1};
            evs[i++] = {{0,0}, EV_KEY, BTN_TOOL_FINGER, 1};
            // write(proxy->v_fd, &btn, sizeof(btn));
        }
    } else {
        evs[i++] = {{0,0}, EV_ABS, ABS_MT_TRACKING_ID, -1};
        // 如果物理手指也为0，补发 BTN_TOUCH 0
        if (proxy->has_phy_finger == 0) {
            evs[i++] = {{0,0}, EV_KEY, BTN_TOUCH, 0};
            evs[i++] = {{0,0}, EV_KEY, BTN_TOOL_FINGER, 0};
            // write(proxy->v_fd, &btn, sizeof(btn));
        }
    }
    evs[i++] = {{0,0}, EV_SYN, SYN_REPORT, 0};
    evs[i++] = {{0,0}, EV_ABS, ABS_MT_SLOT, sinfo.value};
    for(int j=0; j<i; j++) write(proxy->v_fd, &evs[j], sizeof(struct input_event));
}

void run_proxy_loop(TouchProxy* proxy) {
    struct input_event ev;
    while (read(proxy->phys_fd, &ev, sizeof(ev)) > 0) {
        // 核心过滤逻辑：防止物理抬起信号打断虚拟触摸
        if (ev.type == EV_KEY && (ev.code == BTN_TOUCH || ev.code == BTN_TOOL_FINGER)) {
            if (ev.value == 1) proxy->has_phy_finger=1;
            if (ev.value == 0) proxy->has_phy_finger=0;
            
            if (ev.value == 0 && proxy->virtual_active) {
					 printf("Ignoring EVKEY %d val %d\n",ev.code,ev.value);
                continue; // 丢弃该抬起事件，维持 BTN_TOUCH 为 1
            }
        }
        // 转发所有其他事件（包括 Slot 数据、坐标等）
        write(proxy->v_fd, &ev, sizeof(ev));
    }
}








/*
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
*/
