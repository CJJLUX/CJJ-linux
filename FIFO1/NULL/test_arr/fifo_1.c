#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <unistd.h>  
#include <pthread.h>
#include <sys/time.h>
#include "fifo_1.h"

#define min(x, y) ((x) < (y) ? (x) : (y))

unsigned int fifo_get(struct cycle_buffer *fifo, unsigned char *buf, int len, int flag)  
{
	int right_have;
	int fifo_len;
	
	if(flag){
   		if(fifo->len_flag > fifo->len_num){
	   	  fifo_len = 0;
		  printf("len_flag > len_num\n");
		}else{
			 fifo_len = fifo->len_buf[fifo->len_flag];
			 if(len < fifo_len){
		  printf("len not\n");
			 	return -2;
			 } 
	   	fifo->len_flag += 1;
		}
	}

	right_have = min(fifo_len, fifo->size - fifo->out);
	if(right_have == fifo_len){
		memcpy(buf, fifo->buf + fifo->out, fifo_len);
		fifo->out += fifo_len;
	}else{
		memcpy(buf, fifo->buf + fifo->out, right_have);
		memcpy(buf + right_have, fifo->buf, len - right_have);
		fifo->out = fifo_len - right_have;
	}
	fifo->payload -= fifo_len;
	
    return fifo_len;  
}  

unsigned int fifo_put(struct cycle_buffer *fifo, unsigned char *buf, int len, int flag)  
{
	int right_have;
	
	if(flag){
  		*(fifo->len_buf + fifo->len_num) = len;
		fifo->len_num += 1;
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
		memcpy(fifo->buf + fifo->in, buf, right_have);
		memcpy(fifo->buf, buf + right_have, len - right_have);
		fifo->in = len - right_have;
	}
	fifo->payload += len;

	return len;
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


