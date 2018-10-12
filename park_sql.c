
#include "park_sql.h"
#include "sig_list.h"
#include "timing.h"
// #include "sqlite3.h"

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>


extern ListNode *tmp_list;		// main中的卡号队列
extern pthread_mutex_t mutex;	// mian中的线程锁
// 上锁：pthread_mutex_lock(&mutex)
// 解锁：pthread_mutex_unlock(&mutex)

struct sql_and_num
{
	sqlite3 *pdb;
	char num[64];
};


void cre_pshow(sqlite3 *parksql, unsigned int number, long tim);


// 初始化数据库
int init_sql(const char *db_path, sqlite3 **parksql)
{
	if (sqlite3_open(db_path, parksql) != SQLITE_OK)
	{
		fprintf(stderr, "sqlite3_open fail\n");
		return -1;
	}
	printf("open sql done\n");

	// 创建两张表
	if (sqlite3_exec(*parksql, "create table if not exists user(name text not null, carid text not null unique, cardid text not null unique);", NULL, NULL, NULL) != SQLITE_OK)
	{
		fprintf(stderr, "create table [user] faill\n");
		sqlite3_close(*parksql);
		return -1;
	}

	if (sqlite3_exec(*parksql, "create table if not exists time(cardid text not null unique, time text not null);", NULL, NULL, NULL) != SQLITE_OK)
	{
		fprintf(stderr, "create table [time] fail\n");
		sqlite3_close(*parksql);
		return -1;
	}
	
	return 0;
}

void dele_sql(sqlite3 *parksql)
{
	sqlite3_close(parksql);
}


int get_item_count(sqlite3 *parksql, const char *table)
{
	if (parksql == NULL)
	{
		fprintf(stderr, "get_item_count: parksql is NULL\n");
		return -1;
	}
	if (table == NULL)
	{
		fprintf(stderr, "get_item_count:table is number\n");
		return -1;
	}

	char sql[bs128] = {0};	/* sql查询语句 	*/
	char **result = NULL;	/* 存放结果 		*/
	int row = 0;			/* 结果数 		*/
	int col = 0; 			/* 字段数 		*/
	char *errmsg = NULL;	/* 错误信息		*/

	// 生成查询语句：
	sprintf(sql, "select * from %s", table);

	if (sqlite3_get_table(parksql, sql, &result, &row, &col, &errmsg) != SQLITE_OK)	
	{
		fprintf(stderr, "get_item_count:sqlite3_get_table fail:%s\n", sqlite3_errmsg(parksql));
		sqlite3_free(errmsg);
		return -1;
	}

	sqlite3_free_table(result);
	return row;
}


// 添加用户
void * add_user(void *arg)
{
	// printf("reading card...\n");
	// unsigned int id = ();		/* 获取到cardid */
	struct sql_and_num *tmp = (struct sql_and_num *)arg;
	sqlite3 *parksql = tmp ->pdb;
	char cardid[64] = {0};
	strcpy(cardid, tmp ->num);
	free(tmp);

	if (in_table(parksql, "user", cardid))
	{
		// 显示信息
		return (void *)0;
	}
	else
	{
		printf("无用户信息，是否添加用户信息？(y/n)");
		char ret = 'z';
		scanf("%c", &ret);
		if (ret != 'y')
		{
			return (void *)0;
		}
	}


	// unsigned int id = 0;
	// printf("input card id:");
	// scanf("%d", &id);
	// if (id < 0)
	// {
	// 	fprintf(stderr, "add_user:get_card fail\n");
	// 	return -1;
	// }
	// char cardid[64] = {0};
	// sprintf(cardid, "%x", id);			/* 转成text数据 */

	// 获取用户信息并写进table
	char name_buf[64] = {0};			/* 姓名 		*/
	char carid[64] = {0};				/* 车牌号 	*/
	printf("input name:");
	scanf("%s", name_buf);
	printf("input car number:");
	scanf("%s", carid);

	char sql[128] = {0};
	sprintf(sql, "INSERT INTO user VALUES (\"%s\", \"%s\", \"%s\");", name_buf, carid, cardid);

	char *errmsg = NULL;				/* 保存错误信息 */
	int ret = sqlite3_exec(parksql, sql, NULL, NULL, &errmsg);
	if(ret != SQLITE_OK)				/* 插入成功 */
	{
		fprintf(stderr, "add_user:INSERT fail:%d\n", ret);
		fprintf(stderr, "err:%s\n", sqlite3_errmsg(parksql));
		sqlite3_free(errmsg);
		return (void *)0;
	}
	if (errmsg)
	{
		sqlite3_free(errmsg);
	}
}


// 判断是否在time中
bool in_table(sqlite3 *parksql, const char *table, const char *cardid)
{
	if (parksql == NULL || cardid == NULL)
	{
		fprintf(stderr, "in_table: parksql or carid is NULL\n");
		return false;
	}

	char sql[bs128] = {0};	/* sql查询语句 	*/
	char **result = NULL;	/* 存放结果 		*/
	int row = 0;			/* 结果数 		*/
	int col = 0; 			/* 字段数 		*/
	char *errmsg = NULL;	/* 错误信息		*/

	// 生成查询语句：
	sprintf(sql, "select * from %s where cardid is \"%s\";", table, cardid);

	if (sqlite3_get_table(parksql, sql, &result, &row, &col, &errmsg) != SQLITE_OK)	
	{
		fprintf(stderr, "get_item_count:sqlite3_get_table fail:%s\n", sqlite3_errmsg(parksql));
		sqlite3_free(errmsg);
		return -1;
	}
	sqlite3_free_table(result);

	if (row == 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}


// 车辆进出
int parking(sqlite3 *parksql, unsigned int number)
{
	if (parksql == NULL)
	{
		fprintf(stderr, "parking: parksql is null\n");
		return -1;
	}

	char cardid[64] = {0};			/* 转换成text */
	sprintf(cardid, "%x", number);

	if (in_table(parksql, "time", cardid))	/*   卡号在time表中，计算时间，删除数据 		*/
	{
		
		char sql[bs128] = {0};	/* sql查询语句 	*/
		char **result = NULL;	/* 存放结果 		*/
		int row = 0;			/* 结果数 		*/
		int col = 0; 			/* 字段数 		*/
		char *errmsg = NULL;	/* 错误信息		*/
		long _time = 0;	/* 进场时间		*/

		// 生成查询语句,查询进场时间：
		sprintf(sql, "select * from time where cardid is \"%s\";", cardid);
		if (sqlite3_get_table(parksql, sql, &result, &row, &col, &errmsg) != SQLITE_OK)	
		{
			fprintf(stderr, "get_item_count:sqlite3_get_table fail:%s\n", sqlite3_errmsg(parksql));
			sqlite3_free(errmsg);
			return -1;
		}else
		{
			char time_in[16] = {0};	
			strcpy(time_in, result[col + 1]);
			// printf("res:%s\n", time_in);
			sscanf(time_in, "%ld",&_time);
			// printf("%ld\n", _time);
		}
		if (result){ sqlite3_free_table(result);}

		// 从表上删除数据
		sprintf(sql, "delete from time where cardid is \"%s\";", cardid);
		if (sqlite3_exec(parksql, sql, NULL, NULL, &errmsg) != SQLITE_OK)
		{
			fprintf(stderr, "parking:delete fail:%s\n", sqlite3_errmsg(parksql));
			if (errmsg){
				sqlite3_free(errmsg);}
			return -1;
		}

		// 获取当前时间
		time_t tim;
		time(&tim);

		long t = tim - _time;
		printf("time:%ld\n", t);
		cre_pshow(parksql, number, t);	/* 展示车主信息 */
		return t;
	}
	else	/* 卡号不在time表中，插入数据 */
	{

		// 创建线程询问添加user车主信息
		struct sql_and_num *tmp_arg = (struct sql_and_num *)malloc(sizeof(struct sql_and_num));
		tmp_arg ->pdb = parksql;
		memset(&tmp_arg ->num, 0, 64);
		strcpy(tmp_arg ->num, cardid);

		pthread_t pid = 0;
		pthread_create(&pid, NULL, add_user, (void *)tmp_arg);
		
		cre_pshow(parksql, number, -1);	/* 展示车主信息,-1表示入库车辆，无时间 */

		// 获取当前时间
		char tim[16] = {0};
		time_t t;
		time(&t);
		sprintf(tim, "%ld", t);

		
		char sql[bs128] = {0};
		sprintf(sql, "INSERT INTO time VALUES (\"%s\", \"%s\");", cardid, tim);
		printf("sql:%s\n", sql);
		char *errmsg = NULL;
		if (sqlite3_exec(parksql, sql, NULL, NULL, &errmsg) != SQLITE_OK)
		{
			fprintf(stderr, "parking:INSERT fail:%s\n", sqlite3_errmsg(parksql));
			sqlite3_free(errmsg);
			return -1;
		}
		return -2;	/* 插入成功 */
	}
}


/**
 * 生成车主信息并创建线程进行显示
 * @param number 卡号
 * @param tim    时长
 */
void cre_pshow(sqlite3 *parksql, unsigned int number, long tim)
{
	if (parksql == NULL)
	{
		fprintf(stderr, "cre_pshow:parksql is NULL\n");
		return;
	}

	struct park_info{
		char name[64];
		char carid[64];
		char parktime[64];
		char cost[64];
		int  ioflag;
	} *info = NULL;		/* 保存车主信息的结构体 */
	info = (struct park_info *)malloc(sizeof(struct park_info));
	if (info == NULL)
	{	fprintf(stderr, "malloc park_info fail\n");
	}
	memset(info, 0, sizeof(struct park_info));
	if (tim == -1)	{info ->ioflag = -1;}
	else	{info ->ioflag = 1;}

	// 在user表上查询信息
	char sql[bs128] = {0};	/* sql查询语句 	*/
	char **result = NULL;	/* 存放结果 		*/
	int row = 0;			/* 结果数 		*/
	int col = 0; 			/* 字段数 		*/
	char *errmsg = NULL;	/* 错误信息		*/	

	// 生成查询语句,查询姓名和车牌：
	sprintf(sql, "select * from user where cardid is \"%x\";", number);
	if (sqlite3_get_table(parksql, sql, &result, &row, &col, &errmsg) != SQLITE_OK)	
	{
		fprintf(stderr, "get_item_count:sqlite3_get_table fail:%s\n", sqlite3_errmsg(parksql));
		sqlite3_free(errmsg);
		return ;
	}else
	{
		sprintf(info ->name,  "车主:%s", result[col]);		/* 姓名 */
		sprintf(info ->carid, "车牌号:%s", result[col + 1]);	/* 车牌 */
	}
	if (result){ sqlite3_free_table(result);}

	// 计算时间和费用
	if (tim >= 0)
	{
		int s = tim%60;			/* 秒数 */
		int m = (tim/60)%60;	/* 分钟 */
		int h = tim/60/60;		/* 小时 */

		sprintf(info ->parktime, "时长:%d时%d分%d秒", h, m, s);	

		int cost = tim * 2;		/* 1sec2yuan */
		// int cost = ((tim/60)%60) / 5;	/* 5min1yuan */
		sprintf(info ->cost, "费用:%d元", cost);
	}
	else
	{
		strcpy(info ->parktime, "时长:入场");
		strcpy(info ->cost, "费用:暂无");
	}

	// 创建线程显示 
	pthread_t pid = 0;
	pthread_create(&pid, NULL, show_park_info, (void *)info);

}