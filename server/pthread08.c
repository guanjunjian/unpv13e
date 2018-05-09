#include	"unpthread.h"
#include	"pthread08.h"

void
thread_make(int i)
{
	void	*thread_main(void *);

	Pthread_create(&tptr[i].thread_tid, NULL, &thread_main, (void *) i);
	return;		/* main thread returns */
}

void *
thread_main(void *arg)
{
	int		connfd;
	void	web_child(int);

	printf("thread %d starting\n", (int) arg);
	for ( ; ; ) {
		//获取clifd的互斥访问
    	Pthread_mutex_lock(&clifd_mutex);
    	//如果已经没有需要处理的已连接套接字
		while (iget == iput)
			//将该进程睡眠于条件变量
			//如果主线程接受一个连接，则会释放条件变量
			//从而唤醒睡眠在该条件变量上的线程
			Pthread_cond_wait(&clifd_cond, &clifd_mutex);
		//取出一个已连接套接字
		connfd = clifd[iget];	/* connected socket to service */
		if (++iget == MAXNCLI)
			iget = 0;
		Pthread_mutex_unlock(&clifd_mutex);
		tptr[(int) arg].thread_count++;
		//线程处理已连接套接字
		web_child(connfd);		/* process request */
		Close(connfd);
	}
}
