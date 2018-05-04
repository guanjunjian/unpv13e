#include	"unp.h"

static void	recvfrom_int(int);
static int	count;

void
dg_echo(int sockfd, SA *pcliaddr, socklen_t clilen)
{
	int			n;
	socklen_t	len;
	char		mesg[MAXLINE];

	//注册SIGINT信号的行为
	Signal(SIGINT, recvfrom_int);

	n = 220 * 1024;
	//设置缓冲区大小为240KB
	Setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &n, sizeof(n));

	for ( ; ; ) {
		len = clilen;
		Recvfrom(sockfd, mesg, MAXLINE, 0, pcliaddr, &len);

		count++;
	}
}

//当服务器进程收到SIGINT信号时，输出count值
static void
recvfrom_int(int signo)
{
	printf("\nreceived %d datagrams\n", count);
	exit(0);
}
