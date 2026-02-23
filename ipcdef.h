#ifndef _IPCDEF_H
#define _IPCDEF_H

#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>

// 坐标存储结构体，需设为 volatile 或由互斥锁保护以防并发冲突
struct ClickCoord {
    int x;
    int y;
};

// 定义通信信号
#define SIG_UPDATE_COORD SIGUSR1


#endif
