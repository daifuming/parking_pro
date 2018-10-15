#ifndef TS_TOUCH_H
#define TS_TOUCH_H

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>


//event0结构体
struct TsDevice
{
	int fd;
	struct input_event samp;
};


//初始化触摸屏设备
struct TsDevice* init_ts(const char *device);
bool destroy_ts(struct TsDevice* ts);

// 
// struct Point get_xy(struct TsDevice* ts);



#endif//TS_TOUCH_H 
