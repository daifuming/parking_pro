#ifndef TIMING_H_
#define TIMING_H_


struct LcdDevice *init_lcd_timing(const char *device);

/**
 * @param x:绘制矩形的左上x坐标
 * @param y:绘制矩形的左上y坐标
 * @param lcd:已经初始化的lcd结构体指针
 * @return	空
 */
void * start_timing(void *arg);


// 显示车主信息线程函数
void *show_park_info(void *arg);


void show_msg_nof(int x, int y, char *msg);




#endif