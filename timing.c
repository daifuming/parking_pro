#include "bitmap.h"
#include "font.h"
#include "timing.h"
#include "lcd_info.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <stdlib.h>
#include <linux/fb.h>
#include <math.h>
#include <sys/ioctl.h>
#include <time.h>
#include <pthread.h>

#define PLAY_WEL "aplay /massstorage/welcome.wav &"	// 播放欢迎语音
#define PLAY_THK "aplay /massstorage/thanks.wav &"	// 播放谢谢语音

extern struct LcdDevice *lcd;	// main中的lcd屏
extern int park_space;			// mian中的车位信息


//初始化Lcd
struct LcdDevice *init_lcd_timing(const char *device)
{
	struct LcdDevice* lcd = malloc(sizeof(struct LcdDevice ));
	if(lcd == NULL) return NULL;
	//1打开设备
	lcd->fd = open(device, O_RDWR);
	if(lcd->fd < 0)
	{
		perror("open lcd fail");
		free(lcd);
		return NULL;
	}

	//2.获取lcd设备信息
	struct fb_var_screeninfo info; //存储lcd信息结构体--在/usr/inlucde/linux/fb.h文件中定义
	int ret = ioctl(lcd->fd, FBIOGET_VSCREENINFO, &info);
	if(ret < 0)
	{
		perror("request fail");
	}

	lcd->width = info.xres;
	lcd->height = info.yres;
	lcd->pixByte = info.bits_per_pixel/8;//每一个像素占用的字节数
	
	//映射
	lcd->mp = mmap(NULL, lcd->width*lcd->height*lcd->pixByte, 
				    PROT_READ|PROT_WRITE,MAP_SHARED, lcd->fd, 0);
	if(lcd->mp == (void *)-1)
	{
		perror("mmap fail");		
	}
	//给lcd设置默认颜色
	lcd->color = 0x000ff00f;
	
	return lcd;
}


void set_time(char *date_buf, char *time_buf)
{
	if (date_buf == NULL || time_buf == NULL)
	{
		printf("str is NULL\n");
		return;
	}

	time_t tim;
	time(&tim);
	// printf("%s\n", ctime(&tim));
	struct tm *t = gmtime(&tim);
	/* 获取到当前的时间，生成字符串 */

	sprintf(time_buf, "%02d:%02d:%02d", t ->tm_hour, t ->tm_min, t ->tm_sec);
	sprintf(date_buf, "%d年%d月%d日 ", t ->tm_year+1900, t ->tm_mon+1, t ->tm_mday);
	// 拼接星期
	switch(t ->tm_wday)
	{
		case 0:	strcat(date_buf, "星期日"); break;
		case 1:	strcat(date_buf, "星期一"); break;
		case 2:	strcat(date_buf, "星期二"); break;
		case 3:	strcat(date_buf, "星期三"); break;
		case 4:	strcat(date_buf, "星期四"); break;
		case 5:	strcat(date_buf, "星期五"); break;
		case 6:	strcat(date_buf, "星期六"); break;
	}
}


void *start_timing(void *arg)
{
	
	int x = ((struct Point *)arg) ->x;
	int y = ((struct Point *)arg) ->y;
	free(arg);
	// 打开字体
	font *f = fontLoad("/usr/share/fonts/DroidSansFallback.ttf");
	fontSetSize(f, 32);

	char date_buf[128] = {0};	/* 存储日期：2018年10月10日 星期三 */
	char time_buf[128] = {0};	/* 存储时间：15：19：40 */

	while(1)
	{	
		bitmap *bm = createBitmap(288, 50, 4);
		bitmap *bm_time = createBitmap(288, 50, 4);

		
		set_time(date_buf, time_buf);
		// printf("%s, %s\n", date_buf, time_buf);
		fontPrint(f, bm, 20, 10, date_buf, getColor(255, 255, 0, 0), 0);
		fontPrint(f, bm_time, 90, 10, time_buf, getColor(255, 255, 0, 0), 0);

		/*显示日期 					y 					x   */
		unsigned int *p = lcd->mp + y*lcd ->width + x;
		for(int i=0; i<50; i++)
		{
			for(int j=0; j<288; j++)
			{
				memcpy(p+j, bm->map+j*4+i*288*4, 4);
			}
			p+=800;
		}
		/* 显示时间 */
		p = lcd->mp + (y+50)*lcd ->width + x;
		for(int i=0; i<50; i++)
		{
			for(int j=0; j<288; j++)
			{
				memcpy(p+j, bm_time->map+j*4+i*288*4, 4);
			}
			p+=800;
		}
		destroyBitmap(bm);
		destroyBitmap(bm_time);
		sleep(1);
	}
	
	// 关闭字体
	fontUnload(f);
	// 关闭bitmap
}


void show_msg(font *f, int x, int y, char *msg)
{
	bitmap *bm = createBitmap(288, 50, 4);		
	fontPrint(f, bm, 20, 10, msg, getColor(255, 255, 0, 0), 0);
	/*显示日期 					y 					x   */
	unsigned int *p = lcd->mp + y*lcd ->width + x;
	for(int i=0; i<50; i++)
	{
		for(int j=0; j<288; j++)
		{
			memcpy(p+j, bm->map+j*4+i*288*4, 4);
		}
		p+=800;
	}destroyBitmap(bm);
}

void show_msg_with_leng(int x, int y, char *msg, int leng)
{
	font *f = fontLoad("/usr/share/fonts/DroidSansFallback.ttf");
	fontSetSize(f, 32);
	bitmap *bm = createBitmap(leng, 50, 4);		
	fontPrint(f, bm, 20, 10, msg, getColor(255, 255, 0, 0), 0);
	/*显示日期 					y 					x   */
	unsigned int *p = lcd->mp + y*lcd ->width + x;
	for(int i=0; i<50; i++)
	{
		for(int j=0; j<leng; j++)
		{
			memcpy(p+j, bm->map+j*4+i*leng*4, 4);
		}
		p+=800;
	}destroyBitmap(bm);
	fontUnload(f);

}


void *show_park_info(void *arg)
{
	struct park_info{
		char name[64];
		char carid[64];
		char parktime[64];
		char cost[64];
		char local[64];
		int  ioflag;
	} *info = (struct park_info *)arg;		/* 保存车主信息的结构体 */

	// 打开字体
	font *f = fontLoad("/usr/share/fonts/DroidSansFallback.ttf");
	fontSetSize(f, 32);

	// 显示车主信息
	show_msg(f, 400, 100, info ->name);
	show_msg(f, 400, 150, info ->carid);
	show_msg(f, 400, 200, info ->parktime);
	show_msg(f, 400, 250, info ->cost);
	show_msg(f, 100, 250, info ->local);

	// 更新车位信息
	char ps_msg[64] = {0};
	park_space += info ->ioflag;
	sprintf(ps_msg, "     剩余车位:%6d 个", park_space);
	show_msg(f, 100, 200, ps_msg);

	// 关闭字体
	fontUnload(f);

	free(info);

	// 播放语音
	// if (info ->ioflag == -1)
	// 	system(PLAY_WEL);
	// else
	// 	system(PLAY_THK);
}

