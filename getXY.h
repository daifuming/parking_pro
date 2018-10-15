#ifndef GETXY_H
#define GETXY_H
#include "point.h"
#include "ts_touch.h"
//手势枚举
// typedef enum Gesture
// {
// 	UP,
// 	DOWN,
// 	LEFT,
// 	RIGHT
// }GESTURE;

struct Gesture
{
	struct Point point_start;
	struct Point point_end;
};

void get_touch_start(struct TsDevice *ts,struct Point *point);
void get_touch_end(struct TsDevice *ts,struct Point *point);

//获取手势
// GESTURE get_gesture(struct TsDevice *ts);

struct Gesture get_gesture(struct TsDevice *ts);

// 返回值：1234/上下左右
int explain_gesture(struct Gesture gst);

bool if_gst_in_rect(struct Gesture gst, struct Rect rect);

#endif