#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <linux/input.h>
#include <unistd.h>
#include <stdbool.h>
#include "getXY.h"
void get_touch_start(struct TsDevice *ts,struct Point *point)
{
	if (ts == NULL) { return; }
	int flag_x = 0,flag_y = 0;
	struct input_event a;
	while(1)
	{
		read (ts ->fd, &a, sizeof(struct input_event));
		if(a.type == EV_ABS && a.code == ABS_X)	//得到数据
		{
			point ->x = a.value;
			flag_x = 1;
		}
		else if(a.type == EV_ABS && a.code == ABS_Y)
		{
			point ->y = a.value;
			flag_y = 1;
		}
		if(flag_y && flag_x && a.type == EV_KEY && a.value == 1)
			break;
	}
}


void get_touch_end(struct TsDevice *ts,struct Point *point)
{
	if (ts == NULL) { return ; }
	int flag_x = 0,flag_y = 0;
	struct input_event a;
	while(1)
	{
		read (ts ->fd, &a, sizeof(struct input_event));
		if(a.type == EV_ABS && a.code == ABS_X)	//得到数据
		{
			point ->x = a.value;
			flag_x = 1;
		}
		else if(a.type == EV_ABS && a.code == ABS_Y)
		{
			point ->y = a.value;
			flag_y = 1;
		}
		if(flag_y && flag_x && a.type == EV_KEY && a.value == 0)
			break;

	}
}

// GESTURE get_gesture(struct TsDevice *ts)
// {
// 	struct Point point_start = {0,0};	//初始xy值
// 	struct Point point_end   = {0,0};	//结束xy值
	
// 	int flag_x = 0,flag_y = 0;		//xy值被读取的标志
// 	int flag_get_end = 0;			//读取start还是end
// 	struct input_event a;			//触摸事件
// 	while(1)
// 	{
// 		read (ts ->fd, &a, sizeof(struct input_event));
// 		if((!flag_get_end) && a.type == EV_ABS && a.code == ABS_X)
// 		{
// 			point_start.x = a.value;
// 			flag_x = 1;
// 		}
// 		else if((!flag_get_end) && a.type == EV_ABS && a.code == ABS_Y)
// 		{
// 			point_start.y = a.value;
// 			flag_y = 1;
// 		}
// 		else if(flag_y && flag_x && a.type == EV_KEY && a.value == 1)
// 		{
// 			flag_x = 0;
// 			flag_y = 0;
// 			flag_get_end = 1;
// 		}
// 		else if(flag_get_end && a.type == EV_ABS && a.code == ABS_X)
// 		{
// 			point_end.x = a.value;
// 			flag_x = 1;
// 		}
// 		else if(flag_get_end && a.type == EV_ABS && a.code == ABS_Y)
// 		{
// 			point_end.y = a.value;
// 			flag_y = 1;
// 		}
// 		else if(flag_y && flag_x && a.type == EV_KEY && a.value == 0)
// 		{
// 			break;
// 		}
// 	}

// 	printf("start :[%d, %d]\n", point_start.x, point_start.y);
// 	printf("end   :[%d, %d]\n", point_end.x,    point_end.y );

// 	int length = point_end.x - point_start.x;
// 	int height = point_end.y - point_start.y;
// 	if (length < 0) length = -length;
// 	if (height < 0) height = -height;

// 	if (length > height)
// 	{
// 		if (point_end.x > point_start.x)
// 			return 3;
// 		else
// 			return 2;
// 	}
// 	else
// 	{
// 		if (point_end.y > point_start.y)
// 			return 1;
// 		else
// 			return 0;
// 	}

// }


struct Gesture get_gesture(struct TsDevice *ts)
{
	struct Point point_start = {0,0};	//初始xy值
	struct Point point_end   = {0,0};	//结束xy值
	
	
	int flag_start_x = 0,flag_start_y = 0;		//xy值被读取的标志
				//读取start还是end
	struct input_event a;			//触摸事件
	int get_start = 1;
	while(1)
	{
		read (ts ->fd, &a, sizeof(struct input_event));
		if (a.type == EV_KEY && a.value && get_start)
		{
			get_start = 0;
		}
		else if(a.type == EV_KEY && (!a.value))
		{
			break;
		}
		
		if (get_start && a.type == EV_ABS && a.code == ABS_X)
		{
			if(a.value != 0)
			{
				flag_start_x = 1;
				point_start.x = a.value;
				point_end.x = a.value;
			}
		}
		else if(get_start && a.type == EV_ABS && a.code == ABS_Y)
		{
			if(a.value != 0)
			{
				flag_start_y = 1;
				point_start.y = a.value;
				point_end.y = a.value;
			}
		}
		else if((!get_start) && a.type == EV_ABS && a.code == ABS_X)
		{
			if(a.value != 0)
				point_end.x = a.value;
		}
		else if((!get_start) && a.type == EV_ABS && a.code == ABS_Y)
		{
			if(a.value != 0)
				point_end.y = a.value;
		}


	}

	// printf("point_start.x:%d, point_start.y:%d\n", point_start.x, point_start.y);
	// printf("point_end.x:%d, point_end.y:%d\n", point_end.x, point_end.y);

	struct Gesture point2;
	point2.point_start.x = point_start.x;
	point2.point_end.x   = point_end.x;
	point2.point_start.y = point_start.y;
	point2.point_end.y   = point_end.y;

	return point2;
}

int explain_gesture(struct Gesture gst)
{
	if(gst.point_start.x == gst.point_end.x && gst.point_start.y == gst.point_end.y)
		return 0;

	int length = gst.point_end.x - gst.point_start.x;
	int height = gst.point_end.y - gst.point_start.y;
	if (length < 0) length = -length;
	if (height < 0) height = -height;

	if (length > height)
	{
		if (gst.point_end.x > gst.point_start.x)
			return 4;	// 向右
		else
			return 3;	// 向左
	}
	else
	{
		if (gst.point_end.y > gst.point_start.y)
			return 2;	// 向下
		else
			return 1;	// 向上
	}
}

bool if_gst_in_rect(struct Gesture gst, struct Rect rect)
{
	if (gst.point_start.x != gst.point_end.x && gst.point_start.y != gst.point_end.y)
		return false;

	return (gst.point_start.x > rect.pointA.x && gst.point_start.x < rect.pointB.x &&
	    gst.point_start.y > rect.pointA.y && gst.point_start.y < rect.pointB.y);

}
