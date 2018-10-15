#ifndef PARK_DRAW_H
#define PARK_DRAW_H


#define start_x 13
#define start_y 324
#define jump_stap 80
#define yellow_car "/massstorage/yellow_car.bmp"
#define null_car "/massstorage/null_car.bmp"
#define nullpark_bg "/massstorage/nullpark_bg.bmp"




/**
 * 绘制从index开始的10个车位图像
 */
void draw_area(int index);

/**
 * 绘制车辆入库动画线程函数，arg传入位置（0-9）
 */
void *parking_draw(void *arg);

void *null_draw(void *arg);

void show_map_index(int index);


// 更新车位图示线程函数
void *update_map(void *arg);

#endif
