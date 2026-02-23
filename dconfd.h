#include "conf.h"
#include "ipcdef.h"
#include <libconfig.h++>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <dirent.h>
#include<sys/inotify.h>


void watch_conf(); // Parent proc is assumed as daemon
