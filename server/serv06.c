/* include serv06 */
#include	"unpthread.h"

int
main(int argc, char **argv)
{
	int				listenfd, connfd;
	void			sig_int(int);
	void			*doit(void *);
	pthread_t		tid;
	socklen_t		clilen, addrlen;
	struct sockaddr	*cliaddr;

	if (argc == 2)
		listenfd = Tcp_listen(NULL, argv[1], &addrlen);
	else if (argc == 3)
		listenfd = Tcp_listen(argv[1], argv[2], &addrlen);
	else
		err_quit("usage: serv06 [ <host> ] <port#>");
	cliaddr = Malloc(addrlen);

	Signal(SIGINT, sig_int);

	for ( ; ; ) {
		clilen = addrlen;
		//主线程阻塞于accept
		connfd = Accept(listenfd, cliaddr, &clilen);

		//当主线程返回一个客户连接时，调用Pthread_create创建一个新线程
		//新线程执行的函数是doit，其参数是“已连接套接字”
		Pthread_create(&tid, NULL, &doit, (void *) connfd);
	}
}

void *
doit(void *arg)
{
	void	web_child(int);

	//让自己脱离，使得主线程不必等待它
	Pthread_detach(pthread_self());
	//调用客户处理函数
	web_child((int) arg);
	//处理完毕后，该线程关闭“已连接套接字”
	Close((int) arg);
	return(NULL);
}
/* end serv06 */

void
sig_int(int signo)
{
	void	pr_cpu_time(void);

	pr_cpu_time();
	exit(0);
}
