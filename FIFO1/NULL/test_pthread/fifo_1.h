#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <unistd.h>  
#include <pthread.h>  

#define BUF_SIZE 		1000

#define SYN_ON			1
#define SYN_OFF			0

struct cycle_buffer{  
    unsigned char *buf;  
    int size;  
    int in;  
    int out;
	int payload;
	int put_ms;
	int get_ms;
    pthread_mutex_t lock;
	pthread_cond_t cond;
//////////////////////////////////////
	int len_in;//数组长度
	int len_out;//当前数组位置
	int len_buf[BUF_SIZE];
};  

int fifo_init(struct cycle_buffer **fifo, int fifo_size, int put_ms, int get_ms);
unsigned int fifo_get(struct cycle_buffer *fifo, unsigned char *buf, int len, int flag);  
unsigned int fifo_put(struct cycle_buffer *fifo, unsigned char *buf, int len, int flag);
void free_fifo(struct cycle_buffer *fifo);

