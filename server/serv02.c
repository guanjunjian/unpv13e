/* include serv02 */
#include	"unp.h"

static int		nchildren;
static pid_t	*pids;

int
main(int argc, char **argv)
{
	int			listenfd, i;
	socklen_t	addrlen;
	void		sig_int(int);
	pid_t		child_make(int, int, int);

	if (argc == 3)
		listenfd = Tcp_listen(NULL, argv[1], &addrlen);
	else if (argc == 4)
		listenfd = Tcp_listen(argv[1], argv[2], &addrlen);
	else
		err_quit("usage: serv02 [ <host> ] <port#> <#children>");
	//需要fork的子进程数
	nchildren = atoi(argv[argc-1]);
	//分配一个存放各个子进程ID的数组，
	//用于父进程即将终止时由main函数终止所有子进程（在sig_int函数中完成）
	pids = Calloc(nchildren, sizeof(pid_t));

	for (i = 0; i < nchildren; i++)
		//生成子进程，并将子进程ID存入数组中
		pids[i] = child_make(i, listenfd, addrlen);	/* parent returns */

	//服务器父进程终止时的信号处理函数
	//该函数完成的事情：
	//1.终止并wait所有子进程
	//2.打印CPU时间
	Signal(SIGINT, sig_int);
	
	//父进程暂停，由子进程进行处理客户的连接和请求
	for ( ; ; )
		pause();	/* everything done by children */
}
/* end serv02 */

/* include sigint */
void
sig_int(int signo)
{
	int		i;
	void	pr_cpu_time(void);

		/* 4terminate all children */
	//给每个子进程发送SIGTERM信号终止它们
	for (i = 0; i < nchildren; i++)
		kill(pids[i], SIGTERM);
	//调用wait汇集所有子进程的资源利用统计
	while (wait(NULL) > 0)		/* wait for all children */
		;
	if (errno != ECHILD)
		err_sys("wait error");
	//打印CPU时间
	pr_cpu_time();
	exit(0);
}
/* end sigint */
