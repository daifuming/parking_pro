#include "park_sql.h"
#include "rfid_gec_getId.h"
#include "sig_list.h"
#include "timing.h"
#include "lcd_info.h"
#include "point.h"

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define LCD "/dev/fb0"


int park_space = 200;			// 车位数200
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

	// 初始化车位数
	int ret = get_item_count(pdb, "time");
	if (ret < 0)
	{
		fprintf(stderr, "init park_space fail\n");
	}
	else
	{
		park_space -= ret;
	}
	char ps_msg[64] = {0};
	sprintf(ps_msg, "     剩余车位:%6d 个", park_space);
	show_msg_nof(100, 200, ps_msg);


	//	创建线程循环读取卡片
	pthread_t pid_readcard = 0;
	ret = pthread_create(&pid_readcard, NULL, get_card, NULL);

	// 创建线程用于显示时间
	pthread_t pid_time = 0;
	struct Point *p = (struct Point *)malloc(sizeof(struct Point));
	p ->x = 100; p ->y = 100;
	ret = pthread_create(&pid_time, NULL, start_timing, (void *)p);



	pthread_join(pid_readcard, NULL);

	dele_sql(pdb);		/* 销毁数据库	*/
	destroy_lcd(lcd);	/* 销毁lcd	*/
	return 0;
}