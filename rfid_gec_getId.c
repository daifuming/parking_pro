#include <stdio.h>
#include <fcntl.h> 
#include <unistd.h>
#include <termios.h> 
#include <sys/types.h>
#include <sys/select.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include "rfid_gec_getId.h"
#include "sig_list.h"
#include "park_sql.h"


extern sqlite3 *pdb;			// main�ж�������ݿ���
extern ListNode *tmp_list;		// main�еĿ��Ŷ���
extern pthread_mutex_t mutex;	// mian�е��߳���
// ������pthread_mutex_lock(&mutex)
// ������pthread_mutex_unlock(&mutex)

volatile unsigned int cardid ;	// ���̱߳�������ȡ����ֵʱʹ���ڴ��ַ��ȡ�����ǼĴ�����ȡ
static struct timeval timeout;
#define DEV_PATH   "/dev/ttySAC1"

/* ���ô��ڲ���:9600���� */
void init_tty(int fd)
{    
	//�������ô��ڵĽṹ��
	struct termios termios_new;
	//����ոýṹ��
	bzero( &termios_new, sizeof(termios_new));
	//	cfmakeraw()�����ն����ԣ���������termios�ṹ�еĸ���������
	cfmakeraw(&termios_new);
	//���ò�����
	//termios_new.c_cflag=(B9600);
	cfsetispeed(&termios_new, B9600);
	cfsetospeed(&termios_new, B9600);
	//CLOCAL��CREAD�ֱ����ڱ������Ӻͽ���ʹ�ܣ���ˣ�����Ҫͨ��λ����ķ�ʽ����������ѡ�    
	termios_new.c_cflag |= CLOCAL | CREAD;
	//ͨ��������������λΪ8λ
	termios_new.c_cflag &= ~CSIZE;
	termios_new.c_cflag |= CS8; 
	//��������żУ��
	termios_new.c_cflag &= ~PARENB;
	//һλֹͣλ
	termios_new.c_cflag &= ~CSTOPB;
	tcflush(fd,TCIFLUSH);
	// �����ý����ַ��͵ȴ�ʱ�䣬������Ҫ����Խ�������Ϊ0
	termios_new.c_cc[VTIME] = 10;
	termios_new.c_cc[VMIN] = 1;
	// �����������/���������
	tcflush (fd, TCIFLUSH);
	//������ú󣬿���ʹ�����º������������
	if(tcsetattr(fd,TCSANOW,&termios_new) )
		printf("Setting the serial1 failed!\n");

}


/*����У���*/
unsigned char CalBCC(unsigned char *buf, int n)
{
	int i;
	unsigned char bcc=0;
	for(i = 0; i < n; i++)
	{
		bcc ^= *(buf+i);
	}
	return (~bcc);
}

//  ����A����

int PiccRequest(int fd)
{
	unsigned char WBuf[128], RBuf[128];
	int  ret, i;
	fd_set rdfd;
	
	memset(WBuf, 0, 128);
	memset(RBuf,1,128);
	WBuf[0] = 0x07;	//֡��= 7 Byte
	WBuf[1] = 0x02;	//����= 0 , ��������= 0x01
	WBuf[2] = 0x41;	//����= 'A'
	WBuf[3] = 0x01;	//��Ϣ����= 1
	WBuf[4] = 0x52;	//����ģʽ:  ALL=0x52
	WBuf[5] = CalBCC(WBuf, WBuf[0]-2);		//У���
	WBuf[6] = 0x03; 	//������־

	FD_ZERO(&rdfd);
	FD_SET(fd,&rdfd);

	write(fd, WBuf, 7);;
	ret = select(fd + 1,&rdfd, NULL,NULL,&timeout);
	switch(ret)
	{
		case -1:
			perror("select error\n");
			break;
		case  0:
			// ��ʱ
			// printf("Request timed out.\n");
			break;
		default:
			ret = read(fd, RBuf, 8);
			if (ret < 0)
			{
				printf("ret = %d, %d\n", ret, errno);
				break;
			}
			if (RBuf[2] == 0x00)	 	//Ӧ��֡״̬����Ϊ0 ������ɹ�
			{
				return 0;
			}
			break;
	}
	return -1;
}


/*����ײ����ȡ��Χ�����ID*/
int PiccAnticoll(int fd)
{
	unsigned char WBuf[128], RBuf[128];
	int ret, i;
	fd_set rdfd;;
	memset(WBuf, 0, 128);
	memset(RBuf,1,128);
	WBuf[0] = 0x08;	//֡��= 8 Byte
	WBuf[1] = 0x02;	//����= 0 , ��������= 0x01
	WBuf[2] = 0x42;	//����= 'B'
	WBuf[3] = 0x02;	//��Ϣ����= 2
	WBuf[4] = 0x93;	//����ײ0x93 --һ������ײ
	WBuf[5] = 0x00;	//λ����0
	WBuf[6] = CalBCC(WBuf, WBuf[0]-2);		//У���
	WBuf[7] = 0x03; 	//������־
	
	FD_ZERO(&rdfd);
	FD_SET(fd,&rdfd);
	write(fd, WBuf, 8);

	ret = select(fd + 1,&rdfd, NULL,NULL,NULL);
	switch(ret)
	{
		case -1:
			perror("select error\n");
			break;
		case  0:
			perror("B-Timeout");
			break;
		default:
			ret = read(fd, RBuf, sizeof(RBuf));
			if (ret < 0)
			{
				printf("ret = %d, %d\n", ret, errno);
				break;
			}
			if (RBuf[2] == 0x00) //Ӧ��֡״̬����Ϊ0 ���ȡID �ɹ�
			{
				cardid = (RBuf[7]<<24) | (RBuf[6]<<16) | (RBuf[5]<<8) | RBuf[4];//����õ��Ŀ�ID
				return 0;
			}
	}
	return -1;
}


void *get_card(void *arg)
{
	int ret, i;
	int fd;
	

	fd = open(DEV_PATH, O_RDWR | O_NOCTTY | O_NONBLOCK);//�Է�����
	if (fd < 0)
	{
		fprintf(stderr, "Open ttySAC1 fail!\n");
		return NULL;
	}
	/*��ʼ������*/
	init_tty(fd);
	timeout.tv_sec = 3; 
	timeout.tv_usec = 0;
	while(1) //�������
	{
		/*�������߷�Χ�Ŀ�   ����A����*/
		if ( PiccRequest(fd)==-1 )
		{
			cardid = 0;
			// continue;
			// printf("The request failed!\n");
		}
		usleep(100000);
		/*���з���ײ����ȡ���߷�Χ������ID   ����B����*/
		if( PiccAnticoll(fd)==-1 )
		{
			cardid = 0;
			// continue;
			// printf("Couldn't get card-id!\n");
		}
		/* ��ȡ����Ƭ��Ϣ */
		else
		{
			if (cardid == 0)
			{
				/* �������Ϊ0���������� */ 
				continue;
			}
			// �˴��Ի�ȡ����cardid����
			printf("card ID = %x\n", cardid);
			parking(pdb, cardid);
			cardid = 0;
			// pthread_mutex_lock(&mutex);
			// in(tmp_list, cardid);			//�Ѷ�ȡ���Ŀ�������
			// pthread_mutex_unlock(&mutex);
			usleep(1000000);

		}
		usleep(100000);
	}
		
	
	close(fd);
	return NULL;	
}
