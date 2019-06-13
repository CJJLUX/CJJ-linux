#include "fifo_1.h"

void show_buf(char *buf, int num)
{
    int i;
    for (i = 0; i < num; i++) {
        printf("%d\n", buf[i]);
    }
    printf("\n");
}

int main()
{
	int ret;
	char testbuf1[3] = {1,2,3};
	char testbuf2[6] = {4,5,6,7,8,9};
	char outbuf[5];
	struct cycle_buffer *fifo;

	ret = fifo_init(&fifo, 9, 0, 0);
	if (ret) {
	    printf("malloc error\n");
	    return 0;
	}

	fifo_put(fifo, testbuf1, 3, SYN_ON);
	if (ret < 0) {
	    printf("put error\n");
	    return 0;
	}
	show_buf(fifo->buf, 3);

	fifo_put(fifo, testbuf2, 6, SYN_ON);
	if (ret < 0) {
	    printf("put error\n");
	    return 0;
	}
	show_buf(fifo->buf + 3, 6);
		
	//show_buf(fifo->buf + 3, 6); //直接打印所有存入BUF的内容,+1 越界
	show_buf(fifo->buf, 9); //直接打印所有存入BUF的内容

//----
  	 fifo_get(fifo, outbuf, 3, SYN_ON); //开启长度同步，len传0 
     show_buf(outbuf, 3);
//----

//	fifo_get(fifo, outbuf);
//	show_buf(outbuf, 6);

	fifo_put(fifo, testbuf1, 3, SYN_ON);
	show_buf(fifo->buf, 3);			

	fifo_get(fifo, outbuf, 6, SYN_ON);		//不是打印不出来，而是len就没那么长
	show_buf(outbuf, 6);

	fifo_get(fifo, outbuf, 3, SYN_ON);
	show_buf(outbuf, 3);

	free_fifo(fifo);

	return 0;
}

