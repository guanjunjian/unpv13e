/* include child_make */
#include	"unp.h"
#include	"child.h"

pid_t
child_make(int i, int listenfd, int addrlen)
{
	//sockfd[0] 父进程使用
	//sockfd[1] 子进程使用
	int		sockfd[2];
	pid_t	pid;
	void	child_main(int, int, int);

	//创建一个字节流管道，是一对Unix域字节流套接字
	Socketpair(AF_LOCAL, SOCK_STREAM, 0, sockfd);

	//父进程
	if ( (pid = Fork()) > 0) {
		//父进程关闭子进程使用的域套接字
		Close(sockfd[1]);
		cptr[i].child_pid = pid;
		cptr[i].child_pipefd = sockfd[0];
		cptr[i].child_status = 0;
		//父进程返回服务器main函数
		return(pid);		/* parent */
	}

	//子进程
	//将流管道的自身的一端复制到标准错误输出
	//这样每个子进程可以通过读写标准错误输出和父进程通信
	Dup2(sockfd[1], STDERR_FILENO);		/* child's stream pipe to parent */
	//子进程关闭父进程使用的域套接字
	Close(sockfd[0]);
	//由于已经将流管道子进程的一端复制到了标准错误输出
	//因此可以关闭sockfd[1]
	Close(sockfd[1]);
	//关闭监听套接字，由父进程进行监听
	Close(listenfd);
	//子进程不返回，进入child_main函数	/* child does not need this open */
	child_main(i, listenfd, addrlen);	/* never returns */
}
/* end child_make */

/* include child_main */
void
child_main(int i, int listenfd, int addrlen)
{
	char			c;
	int				connfd;
	ssize_t			n;
	void			web_child(int);

	printf("child %ld starting\n", (long) getpid());
	for ( ; ; ) {
		//阻塞于read_fd调用，等待父进程传递过来一个已连接套接字描述符
		if ( (n = Read_fd(STDERR_FILENO, &c, 1, &connfd)) == 0)
			err_quit("read_fd returned 0");
		if (connfd < 0)
			err_quit("no descriptor from read_fd");

		web_child(connfd);				/* process request */
		Close(connfd);
		//子进程处理完一个客户后，
		//通过子进程的字节流管道向父进程写回单个字节
		//告知父进程本子进程已可用（闲置）
		//使得该字节流的父进程拥有端变为可读
		//父进程可以更新该子进程的状态变为“可用”
		Write(STDERR_FILENO, "", 1);	/* tell parent we're ready again */
	}
}
/* end child_main */
