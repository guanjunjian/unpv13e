#include	"unpthread.h"
//该头文件包含unp.h和pthread.h，然后定义了包裹pthread_打头的一系列函数

void	*copyto(void *);

//创建的线程需要的两个参数
//连接到服务器的TCP套接字描述符
static int	sockfd;		/* global for both threads to access */
//输入文件的标准I/O库FILE指针
static FILE	*fp;

void
str_cli(FILE *fp_arg, int sockfd_arg)
{
	char		recvline[MAXLINE];
	pthread_t	tid;

	sockfd = sockfd_arg;	/* copy arguments to externals */
	fp = fp_arg;

	//创建线程，新线程ID返回到tid，
	//由新线程执行的函数是copyto，没有参数传递给该线程
	Pthread_create(&tid, NULL, copyto, NULL);

	//主线程调用readline读入套接字数据
	while (Readline(sockfd, recvline, MAXLINE) > 0)
		//主线程将套接字读入数据写入标准输出
		Fputs(recvline, stdout);

	//str_cli函数返回时，回到客户main函数，
	//main函数通过调用exit终止进程
	//进程内的所有线程也就随之终止
}

void *
copyto(void *arg)
{
	char	sendline[MAXLINE];

	//如果该线程从标准输入读到EOF，Fgets返回NULL
	while (Fgets(sendline, MAXLINE, fp) != NULL)
		//将从标准输入读到的输入，写入套接字
		Writen(sockfd, sendline, strlen(sendline));

	//如果该线程从标准输入读到EOF，则向服务器发送FIN
	Shutdown(sockfd, SHUT_WR);	/* EOF on stdin, send FIN */

	//如果该线程从标准输入读到EOF，该线程在main函数的exit调用终止之前结束
	//如果服务器过早终止，则尚未读入EOF的该线程将由main函数调用exit终止
	return(NULL);
		/* 4return (i.e., thread terminates) when EOF on stdin */
}
