#ifndef PARK_SQL
#define PARK_SQL

#include "sqlite3.h"
#include <stdbool.h>

#define PARK_NUM 100
#define bs128 128
#define SQLPATH "/massstorage/database/park.db"
// #define SQLPATH "/mnt/hgfs/pcshare/parking_pro/park.db"
// #define SQLPATH "/home/dai/sqlitedb/park.db"

/**
 * 初始化sqlite3数据库
 * @param  db_path 数据库路径
 * @param  parksql 数据库句柄指针地址。使用二维指针以修改参数的内容
 * @return         成功0，失败-1
 */
int init_sql(const char *db_path, sqlite3 **parksql);

void dele_sql(sqlite3 *parksql);

/**
 * 获取sqlite3数据库中的一张表中的条目数
 * @param  parksql 数据库句柄
 * @param  table   要查询的表
 * @return         表中的条目数
 */
int get_item_count(sqlite3 *parksql, const char *table);

/**
 * 判断某一卡号是否在[time]中
 * @param  parksql 数据库句柄
 * @param  cardid  卡号
 * @return         在则返回true
 */
bool in_table(sqlite3 *parksql, const char *table, const char *cardid);

/**
 * 添加（注册）停车场用户线程函数
 * @param  	parksql struct sql_and_num
			{
				sqlite3 *pdb;
				unsigned int num;
			}
 * @return  
 */
void *add_user(void *arg);

/**
 * 获取车辆停放时长min
 * @param  parksql 数据库句柄
 * @param  number  卡编号
 * @return         正数：停车时长/min
 *                 -2：车辆进入/加入table中
 *                 -1：程序出错
 */
int parking(sqlite3 *parksql, unsigned int number);

/**
 * 通过查询time表获取当前车位使用状态保存到数组park_attr中
 */

int init_park_attr(sqlite3 *parksql);

#endif