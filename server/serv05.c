/* include serv05a */
#include	"unp.h"
#include	"child.h"

static int		nchildren;

int
main(int argc, char **argv)
{
	int			listenfd, i, navail, maxfd, nsel, connfd, rc;
	void		sig_int(int);
	pid_t		child_make(int, int, int);
	ssize_t		n;
	fd_set		rset, masterset;
	socklen_t	addrlen, clilen;
	struct sockaddr	*cliaddr;

	if (argc == 3)
		listenfd = Tcp_listen(NULL, argv[1], &addrlen);
	else if (argc == 4)
		listenfd = Tcp_listen(argv[1], argv[2], &addrlen);
	else
		err_quit("usage: serv05 [ <host> ] <port#> <#children>");

	FD_ZERO(&masterset);
	//打开监听套接字对应的位
	FD_SET(listenfd, &masterset);
	maxfd = listenfd;
	cliaddr = Malloc(addrlen);

	nchildren = atoi(argv[argc-1]);
	//navail用于跟踪当前可用的子进程数
	navail = nchildren;
	//分配Child结构数组的内存空间
	cptr = Calloc(nchildren, sizeof(Child));

		/* 4prefork all the children */
	for (i = 0; i < nchildren; i++) {
		child_make(i, listenfd, addrlen);	/* parent returns */
		//打开各个子进程的字节流管道对应的位
		FD_SET(cptr[i].child_pipefd, &masterset);
		maxfd = max(maxfd, cptr[i].child_pipefd);
	}

	Signal(SIGINT, sig_int);

	for ( ; ; ) {
		rset = masterset;
		//如果navail为0，表示无子进程“闲置”
		//从select的描述符集中关掉与监听套接字对应的位
		//防止父进程再无可用子进程的情况下accept连接
		//内核仍然将这些连接入队，直到达到listen的backlog数为止
		if (navail <= 0)
			FD_CLR(listenfd, &rset);	/* turn off if no available children */
		//父进程使用select监听“监听套接字”和各个子进程的字节流管道
		nsel = Select(maxfd + 1, &rset, NULL, NULL, NULL);

			/* 4check for new connections */
		//监听套接字可读，至少有一个连接准备好accept
		if (FD_ISSET(listenfd, &rset)) {
			clilen = addrlen;
			connfd = Accept(listenfd, cliaddr, &clilen);

			//找出第一个可用的子进程
			for (i = 0; i < nchildren; i++)
				if (cptr[i].child_status == 0)
					break;				/* available */

			//如果遍历完cptr数组也没有可用的子进程，说明子进程不够用
			if (i == nchildren)
				err_quit("no available children");
			//改变被选中的子进程的状态
			cptr[i].child_status = 1;	/* mark child as busy */
			//更新选中子进程处理客户的统计值
			cptr[i].child_count++;
			//可用子进程数减少1
			navail--;

			//把就绪的已连接套接字传递给选中子进程
			n = Write_fd(cptr[i].child_pipefd, "", 1, connfd);
			//父进程关闭已经传送给子进程的这个已连接套接字
			Close(connfd);
			if (--nsel == 0)
				continue;	/* all done with select() results */
		}

			/* 4find any newly-available children */
		for (i = 0; i < nchildren; i++) {
			//child_main函数调用子进程处理完一个客户后，
			//通过子进程的字节流管道向父进程写回单个字节
			//使得该字节流的父进程拥有端变为可读
			if (FD_ISSET(cptr[i].child_pipefd, &rset)) {
				if ( (n = Read(cptr[i].child_pipefd, &rc, 1)) == 0)
					err_quit("child %d terminated unexpectedly", i);
				//将处理完客户的子进程状态更改为可用
				cptr[i].child_status = 0;
				//递增可用子进程计数
				navail++;
				//select中就绪套接字都处理完，则可以提前终止循环
				if (--nsel == 0)
					break;	/* all done with select() results */
			}
		}
	}
}
/* end serv05a */

void
sig_int(int signo)
{
	int		i;
	void	pr_cpu_time(void);

		/* 4terminate all children */
	for (i = 0; i < nchildren; i++)
		kill(cptr[i].child_pid, SIGTERM);
	while (wait(NULL) > 0)		/* wait for all children */
		;
	if (errno != ECHILD)
		err_sys("wait error");

	pr_cpu_time();

	for (i = 0; i < nchildren; i++)
		printf("child %d, %ld connections\n", i, cptr[i].child_count);

	exit(0);
}
