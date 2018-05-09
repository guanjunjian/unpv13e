#include	"unp.h"

static void	sig_alrm(int);

void
dg_cli(FILE *fp, int sockfd, const SA *pservaddr, socklen_t servlen)
{
	int	n;
	char	sendline[MAXLINE], recvline[MAXLINE + 1];

	//建立信号处理函数
	Signal(SIGALRM, sig_alrm);

	while (Fgets(sendline, MAXLINE, fp) != NULL) {

		Sendto(sockfd, sendline, strlen(sendline), 0, pservaddr, servlen);

		//调用recvfrom前设置5秒的超时
		alarm(5);
		if ( (n = recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL)) < 0) {
			//如果fecvfrom被信号处理函数中断，
			//输出一个信息并跳过本次循环执行下一次循环
			if (errno == EINTR)
				fprintf(stderr, "socket timeout\n");
			else
				err_sys("recvfrom error");
		} else {
			//如果读到一行来自服务器的文本，则关掉报警时钟并输出服务器应答
			alarm(0);
			recvline[n] = 0;	/* null terminate */
			Fputs(recvline, stdout);
		}
	}
}

static void
sig_alrm(int signo)
{
	//简单的返回，以发送让recvfrom返回小于0，从而中断被阻塞的recvfrom
	return;			/* just interrupt the recvfrom() */
}
