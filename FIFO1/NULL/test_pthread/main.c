#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "fifo_1.h"

static void *thread_read(void *arg)
{
	char buf[1024];
	unsigned int n, i;
	struct cycle_buffer *fifo;

	fifo = (struct cycle_buffer *)arg;
	
	for (i = 0; i < 1024; i++)
	{
	    printf("thread_read:%d\n", i);
		memset(buf, 0, sizeof(buf));	
		n = fifo_get(fifo, buf, 11, SYN_OFF);
		write(STDOUT_FILENO, buf, n);
		printf("\n");
	}
	printf(" thread_read:%s\n", buf);

	pthread_exit("thread read done");

	return NULL;
}

static void *thread_write(void *arg)
{
	unsigned char buf[] = "hello world";
	struct cycle_buffer *fifo;
	int i;

	fifo = (struct cycle_buffer *)arg;
	
	for (i = 0; i < 1024; i++)
	{
		printf("thread_write%d\n", i);
		fifo_put(fifo, buf, strlen(buf), SYN_OFF);	
	}
	printf(" thread_write:%s\n", buf);

	pthread_exit("thread write done"); 

	return NULL;
}


int main()
{
	pthread_t wtid;
	pthread_t rtid;
	struct cycle_buffer *fifo = NULL;
	int ret;
		
	ret = fifo_init(&fifo, 100, -1, -1);
	if (ret)
	{
		return ret;
	}

	ret = pthread_create(&wtid, NULL, thread_write, (void *)fifo);
	if (ret)
	{
		goto pthread_create_error;	
	}
	
	ret = pthread_create(&rtid, NULL, thread_read, (void *)fifo);
	if (ret)
	{
		goto pthread_create_error;
	}

	pthread_join(wtid, NULL);
	pthread_join(rtid, NULL);

	exit(0);

pthread_create_error:
	free_fifo(fifo);
	return 0;

}

