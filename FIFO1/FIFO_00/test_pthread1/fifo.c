#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <unistd.h>  
#include <pthread.h>
#include <sys/time.h>
#include "fifo.h"

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


unsigned int fifo_get(struct cycle_buffer *fifo, unsigned char *buf, int len)  
{
	int right_have;
	int fifo_len;

	pthread_mutex_lock(&(fifo->lock));

	if(fifo->flag)
	{
   		if(fifo->len_out > fifo->len_in){
	   	  fifo->len_out = 0;
		  fifo_len = *(fifo->len_store + fifo->len_out);
		  printf("len_out > len_in\n");
		}else{
			 fifo_len = *(fifo->len_store + fifo->len_out);
			 }
		if(len < fifo_len){
			printf("len is not\n");
			return -2;
		} 
	   	fifo->len_out += 1;
	}else{
	 	 fifo_len = len;	
 		 }

 	 while(fifo->payload < fifo_len){
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


	right_have = min(fifo_len, fifo->size - fifo->out);
	if(right_have == fifo_len){
		memcpy(buf, fifo->buf + fifo->out, fifo_len);
		fifo->out += fifo_len;
	}else{
		memcpy(buf, fifo->buf + fifo->out, right_have);
		memcpy(buf + right_have, fifo->buf, fifo_len - right_have);
		fifo->out = fifo_len - right_have;
	}
	fifo->payload -= fifo_len;

	pthread_mutex_unlock(&(fifo->lock));
	pthread_cond_signal(&(fifo->cond));	
	
    return fifo_len;  
}  

unsigned int fifo_put(struct cycle_buffer *fifo, unsigned char *buf, int len)  
{
	int right_have;
	
	pthread_mutex_lock(&fifo->lock);
	
	if(fifo->flag){
  		*(fifo->len_store + fifo->len_in) = len;
		fifo->len_in += 1;
	  	if(fifo->len_in >= fifo->size){ 	
			fifo->len_in = 0;
	  	}
	 }

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

	pthread_mutex_unlock(&fifo->lock);
 	pthread_cond_signal(&(fifo->cond));

	return len;
}

int fifo_init(struct cycle_buffer **fifo, int fifo_size, int put_ms, int get_ms, int flag)
{
	*fifo = (struct cycle_buffer *)malloc(sizeof(struct cycle_buffer));  
	if (*fifo == NULL)  
		return -1;

	memset(*fifo, 0, sizeof(struct cycle_buffer));  
	(*fifo)->size = fifo_size;  
	(*fifo)->in = 0;
	(*fifo)->out = 0;
	(*fifo)->payload = 0;
	(*fifo)->len_in = 0;
	(*fifo)->len_out = 0;
	(*fifo)->flag = flag; 
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

	(*fifo)->len_store = (int *)malloc(fifo_size);  
	if ((*fifo)->len_store == NULL)
		{
			printf("free fifo\n");
			free((*fifo));
			return -1;
		}
	else
		memset((*fifo)->len_store, 0, fifo_size);  	

	return 0;
}

void free_fifo(struct cycle_buffer *fifo)
{
	if(fifo)
		{
			free(fifo->len_store);		
			fifo->len_store = NULL;
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


