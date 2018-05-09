/* include serv08 */
#include	"unpthread.h"
#include	"pthread08.h"

static int			nthreads;
pthread_mutex_t		clifd_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t		clifd_cond = PTHREAD_COND_INITIALIZER;

int
main(int argc, char **argv)
{
	int			i, listenfd, connfd;
	void		sig_int(int), thread_make(int);
	socklen_t	addrlen, clilen;
	struct sockaddr	*cliaddr;

	if (argc == 3)
		listenfd = Tcp_listen(NULL, argv[1], &addrlen);
	else if (argc == 4)
		listenfd = Tcp_listen(argv[1], argv[2], &addrlen);
	else
		err_quit("usage: serv08 [ <host> ] <port#> <#threads>");
	cliaddr = Malloc(addrlen);

	nthreads = atoi(argv[argc-1]);
	tptr = Calloc(nthreads, sizeof(Thread));
	iget = iput = 0;

		/* 4create all the threads */
	for (i = 0; i < nthreads; i++)
		thread_make(i);		/* only main thread returns */

	Signal(SIGINT, sig_int);

	for ( ; ; ) {
		clilen = addrlen;
		connfd = Accept(listenfd, cliaddr, &clilen);

		//获取对clifd的互斥访问
		Pthread_mutex_lock(&clifd_mutex);
		//将主线程accept的已连接套接字描述符存入clifd数组中
		clifd[iput] = connfd;
		//如果iput达到最大值，循环使用
		if (++iput == MAXNCLI)
			iput = 0;
		//检查iput下标是否赶上iget下标
		//如果赶上，说明数组不够大
		if (iput == iget)
			err_quit("iput = iget = %d", iput);
		//释放信号量，如果有子进程睡眠于条件变量，则唤醒线程
		Pthread_cond_signal(&clifd_cond);
		//释放对clifd的互斥访问
		Pthread_mutex_unlock(&clifd_mutex);
	}
}
/* end serv08 */

void
sig_int(int signo)
{
	int		i;
	void	pr_cpu_time(void);

	pr_cpu_time();

	for (i = 0; i < nthreads; i++)
		printf("thread %d, %ld connections\n", i, tptr[i].thread_count);

	exit(0);
}
