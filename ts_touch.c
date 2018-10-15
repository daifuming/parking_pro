#include "ts_touch.h"

//初始化触摸屏设备
struct TsDevice* init_ts(const char *device)
{
	struct TsDevice* ts = (struct TsDevice*)malloc(sizeof(struct TsDevice));
	if(ts == NULL) 
	{
		perror("ts malloc fail");
		return NULL;
	}

	ts->fd = open(device, O_RDWR);
	if(ts->fd < 0)
	{
		perror("open ts fail");
		free(ts);
		return NULL;
	}

	return ts;
}


bool destroy_ts(struct TsDevice* ts)
{
	if(ts == NULL) return false;
	if(ts->fd  > 0) close(ts->fd);
	free(ts);
	return true;
}


// struct Point get_xy(struct TsDevice* ts)
// {
// 	struct Point point={0,0};
// 	if(ts == NULL) return point;
// 	bool flag = false;
// 	while(1)
// 	{
// 		int ret = read(ts->fd, &ts->samp, sizeof(struct input_event));
// 		if(ret < 0) continue;

// 		if(ts->samp.type == EV_ABS && ts->samp.code == ABS_X)
// 		{
// 			point.x = ts->samp.value;
// 			flag = true;
// 		}else if(ts->samp.type == EV_ABS && ts->samp.code == ABS_Y)
// 		{
// 			point.y = ts->samp.value;
// 			if(flag == true) break;
// 		}
// 	}
	
// 	return point;
// }


