#include	"unpthread.h"

static void	*doit(void *);		/* each thread executes this function */

int
main(int argc, char **argv)
{
	int				listenfd, connfd;
	pthread_t		tid;
	socklen_t		addrlen, len;
	struct sockaddr	*cliaddr;

	if (argc == 2)
		listenfd = Tcp_listen(NULL, argv[1], &addrlen);
	else if (argc == 3)
		listenfd = Tcp_listen(argv[1], argv[2], &addrlen);
	else
		err_quit("usage: tcpserv01 [ <host> ] <service or port>");

	cliaddr = Malloc(addrlen);

	for ( ; ; ) {
		len = addrlen;
		connfd = Accept(listenfd, cliaddr, &len);
		//传递给doit函数的唯一参数是已连接套接字描述符
		Pthread_create(&tid, NULL, &doit, (void *) connfd);
		//主线程不关闭已连接套接字，而在fork情形下，服务器程序中是要关闭的。
		//因为同一进程内所有线程共享全部描述符，要是主线程调用close，
		//它就会终止相应的连接。创建新线程并不影响已打开描述符的引用计数
	}
}

static void *
doit(void *arg)
{
	//让自身脱离，因为主线程没有理由等待它创建的每个线程
	Pthread_detach(pthread_self());

	str_echo((int) arg);	/* same function as before */
	//线程必须关闭已连接套接字，因为本线程和主线程共享所有的描述符
	//对于fork的情形，子进程就不必close已连接套接字，
	//因为子进程终止时，会把所有打开的描述符在进程终止时都将被关闭
	Close((int) arg);		/* done with connected socket */
	return(NULL);
}
