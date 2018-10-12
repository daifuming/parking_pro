#ifndef RFID_GEC_GETID
#define RFID_GEC_GETID


/* 设置窗口参数:9600速率 */
void init_tty(int fd);


/*计算校验和*/
unsigned char CalBCC(unsigned char *buf, int n);


//  发送A命令
int PiccRequest(int fd);


/*防碰撞，获取范围内最大ID*/
int PiccAnticoll(int fd);


// 循环获取卡号线程函数
void *get_card(void *arg);



#endif