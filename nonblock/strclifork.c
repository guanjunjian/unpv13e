#include	"unp.h"

void
str_cli(FILE *fp, int sockfd)
{
	pid_t	pid;
	char	sendline[MAXLINE], recvline[MAXLINE];

	if ( (pid = Fork()) == 0) {		/* child: server -> stdout */
		while (Readline(sockfd, recvline, MAXLINE) > 0)
			Fputs(recvline, stdout);

		//子进程在套接字上遇到EOF（适时的或过早的）
		//如果过早遇到EOF，子进程向服务器发送一个SIGTERM，
		//告诉父进程停止从标准输入到套接字复制数据
		kill(getppid(), SIGTERM);	/* in case parent still running */
		exit(0);
	}

		/* parent: stdin -> server */
	while (Fgets(sendline, MAXLINE, fp) != NULL)
		Writen(sockfd, sendline, strlen(sendline));

	//标准输入遇到EOF
	//父进程往服务器发送FIN（不能调用close）
	//由于子进程需要继续从服务器到标准输出执行数据复制，
	//直到套接字上遇到EOF
	//因此服务器调用pause让自己进入睡眠状态，等到子进程
	//直到捕获一个信号（子进程来的SIGTERM），尽管父进程不主动捕获，
	//但SIGTERM信号的默认行为是终止进程
	Shutdown(sockfd, SHUT_WR);	/* EOF on stdin, send FIN */
	pause();
	return;
}
