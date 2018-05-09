/* include connect_timeo */
#include	"unp.h"

static void	connect_alarm(int);

int
connect_timeo(int sockfd, const SA *saptr, socklen_t salen, int nsec)
{
	Sigfunc	*sigfunc;
	int		n;

	//为SIGALRM建立一个新的信号处理函数
	//旧的信号处理函数保存在sigfunc中，以便在本函数结束时恢复它
	sigfunc = Signal(SIGALRM, connect_alarm);
	//报警时钟设置成由调用者指定的秒数
	//如果此前已经给本进程设置过报警时钟，那么alarm的返回值是这个报警时钟的当
	//剩余秒数，否则alarm返回值为0
	//如果是已经设置过的情况，需要显示一个警告信息，因为我们推翻了先前设置的报警时钟
	if (alarm(nsec) != 0)
		err_msg("connect_timeo: alarm was already set");

	//如果本调用被中断（即返回EINTR错误），
	//就把errno值设为ETIMEOUT，同时关闭套接字，以防三路握手继续进行
	if ( (n = connect(sockfd, saptr, salen)) < 0) {
		close(sockfd);
		if (errno == EINTR)
			errno = ETIMEDOUT;
	}
	//通过以0位参数调用alarm关闭本进程的报警时钟
	alarm(0);					/* turn off the alarm */
	//恢复原来的信号处理函数
	Signal(SIGALRM, sigfunc);	/* restore previous signal handler */

	return(n);
}

static void
connect_alarm(int signo)
{
	//信号处理函数只是简单的返回，
	//本return语句将中断进程主控制流中那个未决的connect调用，
	//使得它返回一个EINTR错误，当捕获的信号为SIGALARM时，不设置SA_RESTART标志
	return;		/* just interrupt the connect() */
}
/* end connect_timeo */

void
Connect_timeo(int fd, const SA *sa, socklen_t salen, int sec)
{
	if (connect_timeo(fd, sa, salen, sec) < 0)
		err_sys("connect_timeo error");
}
