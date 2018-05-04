/* include child_make */
#include	"unp.h"

pid_t
child_make(int i, int listenfd, int addrlen)
{
	pid_t	pid;
	void	child_main(int, int, int);

	if ( (pid = Fork()) > 0)
		//父进程返回到服务器main函数
		return(pid);		/* parent */

	//子进程，不返回，进入child_main函数
	//该函数是个无限循环，进行客户连接和请求的处理
	child_main(i, listenfd, addrlen);	/* never returns */
}
/* end child_make */

/* include child_main */
void
child_main(int i, int listenfd, int addrlen)
{
	int				connfd;
	void			web_child(int);
	socklen_t		clilen;
	struct sockaddr	*cliaddr;

	cliaddr = Malloc(addrlen);

	printf("child %ld starting\n", (long) getpid());
	//子进程在这个循环中反复，直到父进程被终止
	//父进程发送SIGTERM信号终止所有子进程
	for ( ; ; ) {
		clilen = addrlen;
		//每个子进程调用accept返回一个已连接套接字
		connfd = Accept(listenfd, cliaddr, &clilen);
		
		//处理客户请求
		web_child(connfd);		/* process the request */
		//关闭“已连接套接字”
		Close(connfd);
	}
}
/* end child_main */
