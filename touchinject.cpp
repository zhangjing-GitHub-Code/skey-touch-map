#ifdef __clang__
    #pragma clang diagnostic push 
    #pragma clang diagnostic ignored "-Waddress-of-temporary"
#endif

#include "touchinject.h"

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
int destroy_proxy(TouchProxy *tp){
    tp->safe_stop=1;
	 pthread_kill(tp->loop_th->native_handle(),SIGUSR1);
	 sleep(1);
	 if(tp->safe_stop)
	     pthread_kill(tp->loop_th->native_handle(),SIGSTOP);
    ioctl(tp->phys_fd, EVIOCGRAB, 0);
	 close(tp->phys_fd);
	 //while(tp->safe_stop)sleep(1);
	 close(tp->v_fd);
}

input_absinfo sinfo;
void set_virtual_touch_state(TouchProxy* proxy, int x, int y, bool down) {
    if(axinfo_ok){
				//printf("Trans %d,%d ->",x,y);
        x=imap(x,0,dx_max,absX.minimum, absX.maximum);
        y=imap(y,0,dy_max,absY.minimum, absY.maximum);
				//printf(" %d,%d; map range[%d,%d] -> [%d,%d]\n",x,y,
				//					 dx_max,dy_max,absY.maximum,absY.maximum);
    }
	 ioctl(proxy->phys_fd, EVIOCGABS(ABS_MT_SLOT), &sinfo);

    proxy->virtual_active = down;
    //write(proxy->phys_fd,&EV(EV_ABS, ABS_MT_SLOT, 9),sizeof(input_event));
    //write(proxy->phys_fd,&EV(EV_SYN, SYN_REPORT, 0),sizeof(input_event));
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
	 proxy->pause_fwd=1;
    for(int j=0; j<i; j++) write(proxy->v_fd, &evs[j], sizeof(struct input_event));
	 proxy->pause_fwd=0;
}

void run_proxy_loop(TouchProxy* proxy) {
    struct input_event ev;
    while (read(proxy->phys_fd, &ev, sizeof(ev)) > 0) {
        if(proxy->safe_stop)break;
		  while(proxy->pause_fwd)usleep(1);
        // 核心过滤逻辑：防止物理抬起信号打断虚拟触摸
		  if(ev.type==EV_ABS&& ev.code==ABS_MT_SLOT&&ev.value==9)continue;
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
	 proxy->safe_stop=0;
}

