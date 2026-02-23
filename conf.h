#ifndef _CONF_H
#define _CONF_H

// --- 配置区域 ---

#define KEY_DEV_NAME "pmic_resin"
#define KEYEV_FALLBK "/dev/input/event2" // 注意：需根据 getevent 指令确认音量键节点

#define KTMAP_CONF "/data/adb/ktmap/vupcoord"
#define KTMAP_CONF_DIR "/data/adb/ktmap"
const int MAX_X = 1264;                   // 屏幕分辨率宽
const int MAX_Y = 2780;                   // 屏幕分辨率高
// ----------------

#endif // _CONF_H
