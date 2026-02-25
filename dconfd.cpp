#include "dconfd.h"

using namespace libconfig;

void update_coord(pid_t daemon_pid, short x, short y) {
    union sigval value;
    // 将两个16位坐标打包进一个32位int
    value.sival_int = (x << 16) | (y & 0xFFFF);
    sigqueue(daemon_pid, SIG_UPDATE_COORD, value);
}

void watch_conf(){ // As main of this process
    int ppid=getppid();
    update_coord(ppid,666,888);
	 // int cfd=open(KTMAP_CONF_PATH,O_RDWR|O_CREAT);
	int fd = open(KTMAP_CONF, O_RDONLY|O_CREAT);
	if (fd < 0) {
		perror("Failed to open conf");
		return;
	}
	close(fd);

	int ifd = inotify_init();
	int inaw_r=inotify_add_watch(ifd,KTMAP_CONF_DIR,IN_CLOSE_WRITE|IN_MOVE);
	// printf("IN fd %d aw ret %d\n",ifd,inaw_r);
	char buf[1024];
	inotify_event* ine=(inotify_event *)buf;
    FILE* fp=fopen(KTMAP_CONF,"r");
		//printf(KTMAP_CONF_PATH);
		if(fp!=NULL){
		    printf("Loading config...\n");

		// 重新定位并读取数值 (不关闭 fd)
		    fseek(fp, 0, SEEK_SET);
		    int x,y;
		    fscanf(fp,"%d %d",&x,&y);
		    if(0<=x&&x<=2500&&0<=y&&y<=2500){
		        update_coord(ppid,x,y);
		    }
		    fclose(fp);
		  }

	while (1) {
		// 使用 poll 阻塞，不消耗 CPU
		read(ifd,ine,1023);
		// printf("Read ine, its wd %d\n",ine->wd);
	   fp=fopen(KTMAP_CONF,"r");
		//printf(KTMAP_CONF_PATH);
		if(fp==NULL){
				  printf("WTF NULL!? %d\n",errno);
				  continue;
		  }

		// fseek(fp, 0, SEEK_SET);
				short x,y;
				fscanf(fp,"%d %d",&x,&y);
				if(0<=x&&x<=2500&&0<=y&&y<=2500){
						  update_coord(ppid,x,y);
				}
				fclose(fp);

	}

	close(fd);
	
	// while(1){
//
//				int cfd=open(KTMAP_CONF_PATH,O_RDWR|O_CREAT);
//	 }
}
