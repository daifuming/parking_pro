#include "park_draw.h"
#include "park_sql.h"
#include "lcd_info.h"
#include "timing.h"
#include "getXY.h"
#include "ts_touch.h"

#include <stdio.h>
#include <unistd.h>


extern int park_attr[PARK_NUM];
extern struct LcdDevice *lcd;
extern int map_index;
extern int park_space;



void show_map_index(int index)
{
    char msg[64] = {0};
    sprintf(msg, "%d区", index);
    show_msg_with_leng(3, 260, msg, 75);
}


void draw_area(int index)
{
    if (index > PARK_NUM - 10)
    {
        fprintf(stderr, "index > %d\n", PARK_NUM - 10);
        return;
    }

    show_map_index(index/10);
    draw_bmp_no_zoom(lcd, 0, 313, nullpark_bg);
    for (int i = index; i < index + 10; i ++)
    {
        if (park_attr[i] == 1)
        {
            draw_bmp_no_zoom(lcd, start_x+jump_stap*(i%10), start_y, yellow_car);
        }
    }
}

/**
 * 绘制车辆入库动画线程函数，arg传入位置（0-9）
 */
void *parking_draw(void *arg)
{
    int index = (int)arg;
    if (index >= 10)
    {
        fprintf(stderr, "draw index >= 10!\n");
        return (void *)-1;
    }

    int x = start_x+jump_stap*index;
    draw_bmp_no_zoom(lcd, x, start_y, yellow_car);
    // for (int y = 480; y > start_y; y --)
    // {
    //     draw_bmp_no_zoom(lcd, x, y, yellow_car);
    //     usleep(100000);
    // }
}

void *null_draw(void *arg)
{
    int index = (int)arg;
    if (index >= 10)
    {
        fprintf(stderr, "draw index >= 10!\n");
        return (void *)-1;
    }

    int x = start_x+jump_stap*index;
    draw_bmp_no_zoom(lcd, x, start_y, null_car);
}


void *update_map(void *arg)
{
    struct TsDevice *ts = init_ts("/dev/input/event0");
    while(1)
    {
        // 返回值：1234/上下左右
        int gst = explain_gesture(get_gesture(ts));
        switch (gst)
        {
            case 3: 
                if (map_index == park_space/10)
                {
                    break;
                }
                draw_area(++map_index*10);
                 break;
            case 4: 
                if (map_index == 0)
                {
                    break;
                }
                draw_area(--map_index*10);
        }
    }
    destroy_ts(ts);
}