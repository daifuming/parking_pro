#include "park_sql.h"
#include "rfid_gec_getId.h"
#include "sig_list.h"
#include "timing.h"
#include "lcd_info.h"
#include "point.h"
#include "park_draw.h"

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define LCD "/dev/fb0"


int map_index = 0;
int park_attr[PARK_NUM] = {0};	// 记录停车场状态的数组，PARK_ATTR在park_sql.h宏定义
int park_space = PARK_NUM;		// 车位数100
struct LcdDevice *lcd = NULL;	// lcd结构体
sqlite3 *pdb = NULL;			// 数据库句柄
ListNode *tmp_list = NULL;		// 
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
// 上锁：pthread_mutex_lock(&mutex)
// 解锁：pthread_mutex_unlock(&mutex)


int main(int argc, char const *argv[])
{
	if (init_sql(SQLPATH, &pdb) < 0)	return -1;	/* 初始化数据库 */
	
	// 初始化lcd屏
	lcd = init_lcd(LCD);
	if (lcd == NULL)
	{
		fprintf(stderr, "init_lcd fail\n");
		return -1;
	}
	draw_bmp_no_zoom(lcd, 0, 0, "/massstorage/park_bg.bmp");
	// draw_bmp_no_zoom(lcd, 16, 324, "/massstorage/yellow_car.bmp");
	

	// 初始化车位数
	int ret = get_item_count(pdb, "time");
	if (ret < 0){	fprintf(stderr, "init park_space fail\n");}
	else{	park_space -= ret;	}
	
	char ps_msg[64] = {0};
	sprintf(ps_msg, "     剩余车位:%6d 个", park_space);
	show_msg_nof(100, 200, ps_msg);

	// 初始化车位数组
	init_park_attr(pdb);
	draw_area(map_index);			/* 初始显示0区 */
	
	// 创建线程进行手势识别
	pthread_t pid_gst = 0;
	ret = pthread_create(&pid_gst, NULL, update_map, NULL);
	if (ret < 0) printf("create gst pthread fail\n");
	else printf("create gst pthread success\n");

	//	创建线程循环读取卡片
	pthread_t pid_readcard = 0;
	ret = pthread_create(&pid_readcard, NULL, get_card, NULL);
	if (ret < 0) printf("create readcard pthread fail\n");
	else printf("create readcard pthread success\n");

	// 创建线程用于显示时间
	pthread_t pid_time = 0;
	struct Point *p = (struct Point *)malloc(sizeof(struct Point));
	p ->x = 100; p ->y = 100;
	ret = pthread_create(&pid_time, NULL, start_timing, (void *)p);
	if (ret < 0) printf("create time pthread fail\n");
	else printf("create time pthread success\n");



	// pthread_join(pid_readcard, NULL);
	while(1)
	{sleep(10);}

	dele_sql(pdb);		/* 销毁数据库	*/
	destroy_lcd(lcd);	/* 销毁lcd	*/
	return 0;
}