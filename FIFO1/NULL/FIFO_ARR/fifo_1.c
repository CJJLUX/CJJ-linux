#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <unistd.h>  
#include <pthread.h>
#include <sys/time.h>
#include "fifo_1.h"

#define min(x, y) ((x) < (y) ? (x) : (y))

static void get_timeout_ts(int time_ms, struct timespec *out_ts)
{
	struct timeval now;
	struct timespec ts;
	
	gettimeofday(&now, NULL);

	ts.tv_sec = now.tv_sec;
	ts.tv_nsec = now.tv_usec * 1000;
	ts.tv_sec += time_ms / 1000;
	ts.tv_nsec += (time_ms % 1000) * 1000 * 1000;
	ts.tv_sec += ts.tv_nsec/(1000 * 1000 * 1000);	
	ts.tv_nsec = ts.tv_nsec%(1000 * 1000 * 1000);
	out_ts->tv_sec = ts.tv_sec;
	out_ts->tv_nsec = ts.tv_nsec;
}

unsigned int fifo_get(struct cycle_buffer *fifo, unsigned char *buf, int len, int flag)  
{
	int right_have;
	
	pthread_mutex_lock(&(fifo->lock));

	if(flag){
    	if (fifo->len_flag > fifo->len_num){
	    	  len = 0;
			  printf("len_flag > len_num\n");
	    	}else{
				 len = fifo->len_buf[fifo->len_flag];
	   	 		 fifo->len_flag += 1;
		}
	}

	while(fifo->payload < len){
		if(fifo->get_ms == -1){
			pthread_cond_wait(&(fifo->cond), &(fifo->lock));
		}else{
			struct timespec ts;
			get_timeout_ts(fifo->get_ms, &ts);
			if(0 != pthread_cond_timedwait(&(fifo->cond), &(fifo->lock), &ts))
			{
				pthread_mutex_unlock(&(fifo->lock));
				return -1;
			}
		}
	}

	right_have = min(len, fifo->size - fifo->out);
	if(right_have == len){
		memcpy(buf, fifo->buf + fifo->out, len);
		fifo->out += len;
	}else{
		//要取得>存有的，先取出存有的，再从头取
		//所处位置为从头数剩余量
		memcpy(buf, fifo->buf + fifo->out, right_have);
		memcpy(buf + right_have, fifo->buf, len - right_have);
		fifo->out = len - right_have;
	}
	fifo->payload -= len;
	
	pthread_mutex_unlock(&(fifo->lock));
	pthread_cond_signal(&(fifo->cond));
	
    return len;  
}  

unsigned int fifo_put(struct cycle_buffer *fifo, unsigned char *buf, int len, int flag)  
{
	int right_have;
	int i = 8;
	
	pthread_mutex_lock(&fifo->lock);

	while((fifo->size - fifo->payload) < len){
		if(fifo->put_ms == -1){
			pthread_cond_wait(&(fifo->cond), &(fifo->lock));
		}else{
			struct timespec ts;
			get_timeout_ts(fifo->put_ms, &ts);
			if(0 != pthread_cond_timedwait(&(fifo->cond), &(fifo->lock), &ts))
			{
				pthread_mutex_unlock(&(fifo->lock));
				return -1;
			}
		}
	}

	if(flag){
  		*(fifo->len_buf + fifo->len_num) = len;
		fifo->len_num += 1;
		//1000个数组到头了
	  	if(fifo->len_num >= 1000){
			memset(fifo->len_buf, 0, 1000); 	
			fifo->len_num = 0;
	  	}
	 }

	right_have = min(len, fifo->size - fifo->in);
	if(right_have == len){
		memcpy(fifo->buf + fifo->in, buf, len);
		fifo->in += len;
	}else{
		//len > size-in
		memcpy(fifo->buf + fifo->in, buf, right_have);
		//但是len > 剩余的长度， 那么就接着buf后面的数据，重头继续往里面传
		memcpy(fifo->buf, buf + right_have, len - right_have);
		fifo->in = len - right_have;
	}
	fifo->payload += len;

	pthread_mutex_unlock(&fifo->lock);
	pthread_cond_signal(&(fifo->cond));

	return len;
}

int fifo_valid(struct cycle_buffer *fifo)
{
	return fifo->payload;
}

int fifo_init(struct cycle_buffer **fifo, int fifo_size, int put_ms, int get_ms)
{
	*fifo = (struct cycle_buffer *)malloc(sizeof(struct cycle_buffer));  
	if (*fifo == NULL)  
		return -1;

	memset(*fifo, 0, sizeof(struct cycle_buffer));  
	(*fifo)->size = fifo_size;  
	(*fifo)->in = 0;
	(*fifo)->out = 0;
	(*fifo)->payload = 0;
	(*fifo)->len_num = 0;
	(*fifo)->len_flag = 0;
	(*fifo)->put_ms = put_ms;
	(*fifo)->get_ms = get_ms;
	pthread_mutex_init(&(*fifo)->lock, NULL);
	pthread_cond_init(&(*fifo)->cond, NULL);
	(*fifo)->buf = (unsigned char *)malloc(fifo_size);  
	if ((*fifo)->buf == NULL)
		{
			printf("free fifo\n");
			free((*fifo));
			return -1;
		}
	else
		memset((*fifo)->buf, 0, fifo_size);  	

	return 0;
}

void free_fifo(struct cycle_buffer *fifo)
{
	if(fifo)
		{
			free(fifo->buf);		
			fifo->buf = NULL;
			free(fifo);
			fifo = NULL;
		}
}


void free_clean(struct cycle_buffer *fifo)
{
	if(fifo)
		{
			fifo->in = 0;		
			fifo->out = 0;
			fifo->payload = 0;
		}
}


